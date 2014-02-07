// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2012, The TPIE development team
//
// This file is part of TPIE.
//
// TPIE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
//
// TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
// License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with TPIE. If not, see <http://www.gnu.org/licenses/>

#ifndef __TPIE_PIPELINING_MERGE_SORTER_H__
#define __TPIE_PIPELINING_MERGE_SORTER_H__

#include <tpie/pipelining/sort_parameters.h>
#include <tpie/pipelining/merger.h>
#include <tpie/pipelining/exception.h>
#include <tpie/dummy_progress.h>
#include <tpie/array_view.h>
#include <tpie/blocking_queue.h>
#include <tpie/parallel_sort.h>
#include <boost/random/uniform_01.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <deque>

namespace tpie {

///////////////////////////////////////////////////////////////////////////////
/// Merge sorting consists of three phases.
///
/// 1. Sorting and forming runs
/// 2. Merging runs
/// 3. Final merge and report
///
/// If the number of elements received during phase 1 is less than the length
/// of a single run, we are in "report internal" mode, meaning we do not write
/// anything to disk. This causes phase 2 to be a no-op and phase 3 to be a
/// simple array traversal.
///////////////////////////////////////////////////////////////////////////////
template <typename T, bool UseProgress, typename pred_t = std::less<T> >
class merge_sorter {
public:
        typedef boost::shared_ptr<merge_sorter> ptr;
        typedef progress_types<UseProgress> Progress;

        static const memory_size_type maximumFanout = 250; // This is the max number of runs to merge at a time when running a k-way merge.
        static const memory_size_type bufferCount = 4; // This is the number of buffers to be used during phase 1

        ///////////////////////////////////////////////////////////////////////////////
        /// \brief Create a new merger_sorter object with the given predicate
        ///////////////////////////////////////////////////////////////////////////////
        merge_sorter(pred_t pred = pred_t())
        : m_parameters_set(false)
        , m_evacuated(false)
        , m_pred(pred)
        , m_reporting_mode(REPORTING_MODE_EXTERNAL)
        , m_state(STATE_PARAMETERS)
        , m_runsPushed(0)
        , m_itemsPushed(0)
        , m_itemsPulled(0)
        , m_merger(pred)
        {
                m_parameters.memoryPhase1 = 0;
                m_parameters.memoryPhase2 = 0;
                m_parameters.memoryPhase3 = 0;
        }

        ///////////////////////////////////////////////////////////////////////////
    /// \brief Enable setting run length and fanout manually (for testing
    /// purposes).
    ///////////////////////////////////////////////////////////////////////////
    void set_parameters(memory_size_type runLength, memory_size_type fanout) {
            tp_assert(m_state == STATE_PARAMETERS, "Merge sorting already begun");
            m_parameters.runLength = m_parameters.internalReportThreshold = runLength;
            m_parameters.fanout = m_parameters.finalFanout = fanout;

            m_state = STATE_PARAMETERS;
    }

private:
        void calculate_parameters() {
                tp_assert(m_state == STATE_PARAMETERS, "Merge sorting already begun");

                ///////////////////////////////////////////////////////////////////////////////
                /// Phase 2
                /// The fanout is determined by the size of the merge heap and the stream
                /// memory usage
                ///////////////////////////////////////////////////////////////////////////////
                m_parameters.fanout = calculate_fanout(m_parameters.memoryPhase2);
                if(fanout_memory_usage(m_parameters.fanout) > m_parameters.memoryPhase2) {
                        log_debug() << "Not enough memory for fanout "
                                                << m_parameters.fanout << "! ("
                                                << m_parameters.memoryPhase2 << " < "
                                                << fanout_memory_usage(m_parameters.fanout) << ")\n";
                        
            m_parameters.memoryPhase2 = fanout_memory_usage(m_parameters.fanout);
                }


                ///////////////////////////////////////////////////////////////////////////////
                /// Phase 1
                /// The run length is determined by the number of items we can hold in memory
                /// and the number of buffers we are using
                ///////////////////////////////////////////////////////////////////////////////
                m_parameters.runLength = (m_parameters.memoryPhase1 - file_stream<T>::memory_usage()) / sizeof(T);

                // if we receive less items than internalReportThreshold, internal report mode will be used(no I/O)
                m_parameters.internalReportThreshold = std::min(m_parameters.memoryPhase1, std::min(m_parameters.memoryPhase2, m_parameters.memoryPhase3)) / sizeof(T);
                if(m_parameters.internalReportThreshold > m_parameters.runLength)
                        m_parameters.internalReportThreshold = m_parameters.runLength;

                ///////////////////////////////////////////////////////////////////////////////
                /// Phase 3
                ///////////////////////////////////////////////////////////////////////////////
                m_parameters.finalFanout = calculate_fanout(m_parameters.memoryPhase3);

                if(fanout_memory_usage(m_parameters.finalFanout) > m_parameters.memoryPhase3) {
                        log_debug() << "Not enough memory for fanout "
                                                << m_parameters.finalFanout
                                                << "! (" << m_parameters.memoryPhase3
                                                << " < " << fanout_memory_usage(m_parameters.finalFanout) << ")\n";

                        m_parameters.memoryPhase3 = fanout_memory_usage(m_parameters.finalFanout);
                }

                ///////////////////////////////////////////////////////////////////////////////
                /// Final
                ///////////////////////////////////////////////////////////////////////////////
                m_parameters_set = true;
        }

        static memory_size_type calculate_fanout(memory_size_type memory) {
                // do a binary search to determine the fanout
                memory_size_type l = 2;
                memory_size_type h = maximumFanout+1;

                while(l < h-1) {
                        memory_size_type m = (l+h)/2;
                        //tpie::log_info() << l << " " << m << " " << h << std::endl;
                        memory_size_type usage = fanout_memory_usage(m);
                        if(usage <= memory)
                                l = m;
                        else
                                h = m-1;
                }

                return l;
        }

        static memory_size_type fanout_memory_usage(memory_size_type fanout) {
        	return sizeof(merge_sorter<T, UseProgress, pred_t>)
                	- sizeof(merger<T, pred_t>)
                	+ merger<T, pred_t>::memory_usage(fanout)
                    + file_stream<T>::memory_usage();
        }
public:

        ///////////////////////////////////////////////////////////////////////////
        /// \brief Calculate parameters from given memory amount.
        /// \param m Memory available for phase 2, 3 and 4
        ///////////////////////////////////////////////////////////////////////////
        void set_available_memory(memory_size_type m) {
                m_parameters.memoryPhase1 = m_parameters.memoryPhase2 = m_parameters.memoryPhase3 = m;
                calculate_parameters();
        }

        ///////////////////////////////////////////////////////////////////////////
        /// \brief Calculate parameters from given memory amount.
        /// \param m1 Memory available for phase 1
        /// \param m2 Memory available for phase 2
        /// \param m3 Memory available for phase 3
        ///////////////////////////////////////////////////////////////////////////
        void set_available_memory(memory_size_type m1, memory_size_type m2, memory_size_type m3) {
                m_parameters.memoryPhase1 = m1;
                m_parameters.memoryPhase2 = m2;
                m_parameters.memoryPhase3 = m3;        

                calculate_parameters();
        }

private:
        ///////////////////////////////////////////////////////////////////////////////
        /// \brief Helper for set_phase_?_memory
        ///////////////////////////////////////////////////////////////////////////////
        void maybe_calculate_parameters() {
                if(m_parameters.memoryPhase1 > 0 && m_parameters.memoryPhase2 > 0 && m_parameters.memoryPhase3 > 0) {
                        calculate_parameters();
                }
        }
public:
        ///////////////////////////////////////////////////////////////////////////////
        /// \brief Calculate parameters from given memory amount for phase 1.
        /// \param mem Memory avaiable for phase 1
        ///////////////////////////////////////////////////////////////////////////////
        void set_phase_1_memory(memory_size_type mem) {
                m_parameters.memoryPhase1 = mem;
                maybe_calculate_parameters();
        }

        ///////////////////////////////////////////////////////////////////////////////
        /// \brief Calculate parameters from given memory amount for phase 2.
        /// \param mem Memory avaiable for phase 2
        ///////////////////////////////////////////////////////////////////////////////
        void set_phase_2_memory(memory_size_type mem) {
                m_parameters.memoryPhase2 = mem;
                maybe_calculate_parameters();
        }

        ///////////////////////////////////////////////////////////////////////////////
        /// \brief Calculate parameters from given memory amount for phase 3.
        /// \param mem Memory avaiable for phase 3
        ///////////////////////////////////////////////////////////////////////////////
        void set_phase_3_memory(memory_size_type mem) {
                m_parameters.memoryPhase3 = mem;
                maybe_calculate_parameters();
        }

        void phase1_sort() {
                while(true) {
                        std::vector<T> * run = m_fullBuffers.pop();
                        if(run == NULL) {
                        	m_sortedBuffers.push(NULL);
                        	break;
                        }

                        tpie::parallel_sort(run->begin(), run->end(), m_pred);
                        m_sortedBuffers.push(run);
                }
        }

        void phase1_write() {
                while(true) {
                        std::vector<T> * run = m_sortedBuffers.pop();
                        if(run == NULL) {
                        	break;
                        }

                        temp_file runFile;
                        file_stream<T> out;
                        out.open(runFile, access_read_write);
                        for(typename std::vector<T>::iterator i = run->begin(); i != run->end(); ++i) {
                                out.write(*i);
                        }
                        out.close();

                        m_runFiles.push_back(runFile);
                        run->resize(0);
                        m_emptyBuffers.push(run); // push a new empty buffer
                }
        }

        ///////////////////////////////////////////////////////////////////////////
        /// \brief Initiate phase 1: Formation of input runs.
        ///////////////////////////////////////////////////////////////////////////
        void begin() {
        		log_debug() << "Beginning phase 1" << std::endl;
                tp_assert(m_parameters_set, "Parameters have not been set");
                m_state = STATE_RUN_FORMATION;

                for(memory_size_type i = 0; i < bufferCount; ++i)
                        m_emptyBuffers.push(new std::vector<T>());

                m_SortThread = boost::thread(boost::bind(&merge_sorter::phase1_sort, this));
                m_WriteThread = boost::thread(boost::bind(&merge_sorter::phase1_write, this));
        }

        ///////////////////////////////////////////////////////////////////////////
        /// \brief Push item to merge sorter during phase 1.
        ///////////////////////////////////////////////////////////////////////////
        void push(const T & item) {
        		memory_size_type desired_size = m_parameters.runLength;
        		if(m_runsPushed == 0) desired_size /= 3;
        		else if(m_runsPushed == 1) desired_size /= 2;

                if(m_emptyBuffers.front()->size() == desired_size) {
                        m_fullBuffers.push(m_emptyBuffers.pop());
                        ++m_runsPushed;
                }

                m_emptyBuffers.front()->push_back(item);
               	++m_itemsPushed;
        }

        ///////////////////////////////////////////////////////////////////////////
        /// \brief End phase 1.
        ///////////////////////////////////////////////////////////////////////////
        void end() {
                // Set to internal reporting mode if possible

                if(m_runsPushed == 0 && m_itemsPushed <= m_parameters.internalReportThreshold) {
                        /*log_info() << "Internal reporting mode set. " << std::endl;
                        log_info() << "Capacity: " << m_emptyBuffers.front()->capacity() << std::endl;
                        log_info() << "Size: " << m_emptyBuffers.front()->size() << std::endl;
                        log_info() << "Threshold: " << m_parameters.internalReportThreshold << std::endl;
                        log_info() << "Items pushed: " << m_itemsPushed << std::endl;*/

                        // no buffer has been queued for sorting or writing yet and the number size is small enough.
                        // use internal report mode

                        // TODO: Resize vector

                		m_fullBuffers.push(NULL);
                        tpie::parallel_sort(m_emptyBuffers.front()->begin(), m_emptyBuffers.front()->end(), m_pred);
                        m_reporting_mode = REPORTING_MODE_INTERNAL;
                }
                else {
                        /*log_info() << "External reporting mode set. " << std::endl;
                        log_info() << "Capacity: " << m_emptyBuffers.front()->capacity() << std::endl;
                        log_info() << "Size: " << m_emptyBuffers.front()->size() << std::endl;
                        log_info() << "Threshold: " << m_parameters.internalReportThreshold << std::endl;
                        log_info() << "Items pushed: " << m_itemsPushed << std::endl;*/
                        
                        // use external report mode

                        if(!m_emptyBuffers.front()->empty()) {
                                m_fullBuffers.push(m_emptyBuffers.pop());        
                        }
                        m_fullBuffers.push(NULL);
                }

                m_SortThread.join();
                m_WriteThread.join();
                m_state = STATE_MERGE;

                if(m_reporting_mode == REPORTING_MODE_EXTERNAL) { // clean up
                	while(!m_emptyBuffers.empty())
                		delete m_emptyBuffers.pop();
                }
        }

private:
        ///////////////////////////////////////////////////////////////////////////////
        /// \brief Merges the first n runs in the run deque and pushes a new runfile
        /// to the end
        ///////////////////////////////////////////////////////////////////////////////
        void merge_runs(memory_size_type n, typename Progress::base & pi) {
        		if(n > m_runFiles.size()) {
        			n = m_runFiles.size();
        		}

                // open file streams
                temp_file runFile;
                file_stream<T> out;
                out.open(runFile, access_write);

                array<file_stream<T> > input(n);
                for(memory_size_type i = 0; i < n; ++i) {
                        input[i].open(m_runFiles[i], access_read);
                }

                // setup merger
                m_merger.reset(input);

                // empty the streams and tree
                while(!m_merger.can_pull()) {
                        out.write(m_merger.pull());
                        pi.step();
                }

                for(memory_size_type i = 0; i < n; ++i) {
                    	m_runFiles.pop_front();
                }

                m_runFiles.push_back(runFile);
        }

public:

        ///////////////////////////////////////////////////////////////////////////
        /// \brief Perform phase 2: Performing all merges in the merge tree
        ///////////////////////////////////////////////////////////////////////////
        void calc(typename Progress::base & pi) {
                tpie::log_debug() << "Performing phase 2" << std::endl;

                if(m_reporting_mode == REPORTING_MODE_INTERNAL) {
                        pi.init(1);
                        pi.step();
                        pi.done();
                }
                else {
                        memory_size_type treeHeight = static_cast<int> (
                                ceil(log(static_cast<float>(m_runFiles.size()))) / ceil(log(static_cast<float>(m_parameters.fanout)))
                        ); // the number of merge 'rounds' to be performed.

                        pi.init(treeHeight * m_itemsPushed);

                        while(m_runFiles.size() > m_parameters.finalFanout) {
                                merge_runs(m_parameters.fanout, pi);
                        }

                        pi.done();

                        initialize_final_merger();        
                }
                
                m_state = STATE_REPORT;
                //tpie::log_info() << "Finished phase 2" << std::endl;
        }

        ///////////////////////////////////////////////////////////////////////////////
        /// \brief If in internal reporting mode: write data to disk. The allocated
        /// memory is needed elsewhere
        ///////////////////////////////////////////////////////////////////////////////
        void evacuate() {
                tp_assert(m_state == STATE_MERGE || m_state == STATE_REPORT, "Wrong phase");

                if(m_reporting_mode == REPORTING_MODE_INTERNAL) {
                        m_reporting_mode = REPORTING_MODE_EXTERNAL; // write the buffer to disk and use external reporting mode

                        if(!m_emptyBuffers.front()->empty()) { // write the buffer to disk
                                temp_file runFile;
                                file_stream<T> out;
                                out.open(runFile, access_read_write);
                                for(typename std::vector<T>::iterator i = m_emptyBuffers.front()->begin(); i != m_emptyBuffers.front()->end(); ++i)
                                        out.write(*i);
                                out.close();
                                m_runFiles.push_back(runFile);

                                while(!m_emptyBuffers.empty())
                					delete m_emptyBuffers.pop();
                        }
                }
                else {
                        m_merger.reset();
                }

                m_evacuated = true;
        }

        ///////////////////////////////////////////////////////////////////////////////
        /// \brief Evacuate if we are in the merging state
        ///////////////////////////////////////////////////////////////////////////////
        void evacuate_before_merging() {
                if (m_state == STATE_MERGE)
                        evacuate();
        }

        ///////////////////////////////////////////////////////////////////////////////
        /// \brief Evacuate if we are in the reporting state
        ///////////////////////////////////////////////////////////////////////////////
        void evacuate_before_reporting() {
                if (m_state == STATE_REPORT && (m_reporting_mode == REPORTING_MODE_INTERNAL || m_itemsPulled == 0))
                        evacuate();
        }

        ///////////////////////////////////////////////////////////////////////////////
        /// \brief Reinitialize the merger for phase 3
        ///////////////////////////////////////////////////////////////////////////////
        void initialize_final_merger() {
                log_debug() << "Initialize final merger" << std::endl;
                array<file_stream<T> > inputs(m_runFiles.size());
                for(memory_size_type i = 0; i < m_runFiles.size(); ++i) {
                        //log_info() << "Opening stream " << i << std::endl;
                        inputs[i].open(m_runFiles[i], access_read);
                }

                m_evacuated = false;

                m_merger.reset(inputs);
        }

        ///////////////////////////////////////////////////////////////////////////
        /// \brief In phase 3, return true if there are more items in the final merge
        /// phase.
        ///////////////////////////////////////////////////////////////////////////
        bool can_pull() {
                tp_assert(m_state == STATE_REPORT, "Wrong phase");
                return m_itemsPushed > m_itemsPulled;
        }

        ///////////////////////////////////////////////////////////////////////////
        /// \brief In phase 3, fetch next item in the final merge phase.
        ///////////////////////////////////////////////////////////////////////////
        T pull() {
                tp_assert(m_state == STATE_REPORT, "Wrong phase");
                if(m_reporting_mode == REPORTING_MODE_INTERNAL) {
                        T el = m_emptyBuffers.front()->at(m_itemsPulled++);
                        if(!can_pull()) {
                        	while(!m_emptyBuffers.empty()) 
                        		delete m_emptyBuffers.pop();
                        }
                        return el;
                }
                else {
                        if(m_evacuated) initialize_final_merger();
                        ++m_itemsPulled;
                        return m_merger.pull();
                }
        }

        ///////////////////////////////////////////////////////////////////////////////
        /// \brief Return the number of items in the sorter
        ///////////////////////////////////////////////////////////////////////////////
        stream_size_type item_count() {
                return m_itemsPushed - m_itemsPulled;
        }
public:
        static memory_size_type memory_usage_phase_1(const sort_parameters & params) {
                return sizeof(merge_sorter<T, UseProgress, pred_t>)
                		+ params.runLength * sizeof(T) * bufferCount;
        }

        static memory_size_type minimum_memory_phase_1() {
                // a runlength of 1
                return sizeof(T) + sizeof(merge_sorter<T, UseProgress, pred_t>);
        }

        static memory_size_type memory_usage_phase_2(const sort_parameters & params) {
                return fanout_memory_usage(params.fanout);
        }

        static memory_size_type minimum_memory_phase_2() {
                return fanout_memory_usage(0);
        }

        static memory_size_type memory_usage_phase_3(const sort_parameters & params) {
                return fanout_memory_usage(params.finalFanout);
        }

        static memory_size_type minimum_memory_phase_3() {
                // The minimum amount of memory used is when the fanout is zero
                return fanout_memory_usage(0);
        }

        static memory_size_type maximum_memory_phase_3() {
                // The maximum amount of memory is used when the fanout is the maximum possible fanout
                return fanout_memory_usage(maximumFanout);
        }

        ///////////////////////////////////////////////////////////////////////////////
        /// \brief The memory usage when the sorter is evacuated.
        ///////////////////////////////////////////////////////////////////////////////
        memory_size_type evacuated_memory_usage() const {
                return sizeof(merge_sorter<T, UseProgress, pred_t>) + sizeof(temp_file) * maximumFanout;
        }

        ///////////////////////////////////////////////////////////////////////////
        /// \brief Set upper bound on number of items pushed.
        ///
        /// If the number of items to push is less than the size of a single run,
        /// this method will decrease the run size to that.
        /// This may make it easier for the sorter to go into internal reporting
        /// mode.
        ///////////////////////////////////////////////////////////////////////////
        void set_items(stream_size_type) {
                // TODO: Use for something....
        }
private:
        enum reporting_mode {
                REPORTING_MODE_INTERNAL, // do not write anything to disk. Phase 2 will be a no-op phase and phase 3 is simple array traversal
                REPORTING_MODE_EXTERNAL
        };

        enum state_type {
                STATE_PARAMETERS,
                STATE_RUN_FORMATION,
                STATE_MERGE,
                STATE_REPORT
        };

        sort_parameters m_parameters;
        bool m_parameters_set;
        bool m_evacuated;
        pred_t m_pred;
        reporting_mode m_reporting_mode;
        state_type m_state;
        memory_size_type m_runsPushed;
        memory_size_type m_itemsPushed;
        memory_size_type m_itemsPulled;

        bits::blocking_queue<std::vector<T> *> m_emptyBuffers; // the buffers that are to be consumed by the push method
        bits::blocking_queue<std::vector<T> *> m_fullBuffers; // the buffers that are to be consumed by the sorting thread
        bits::blocking_queue<std::vector<T> *> m_sortedBuffers; // the buffers that are to be consumed by the write thread

        std::deque<temp_file> m_runFiles;

        // phase 1 specific
        boost::thread m_SortThread; // The thread in phase 1 used to sort run formations
        boost::thread m_WriteThread; // The thread in phase 1 used to write run formations to file

        // phase 3 specific
        merger<T, pred_t> m_merger;
};

} // namespace tpie

#endif // __TPIE_PIPELINING_MERGE_SORTER_H__