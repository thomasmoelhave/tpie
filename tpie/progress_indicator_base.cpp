#include "progress_indicator_base.h"

const unsigned int tpie::progress_indicator_base::m_frequency = 20;
double tpie::progress_indicator_base::m_threshold;
bool tpie::progress_indicator_base::m_thresholdComputed = false;

void tpie::progress_indicator_base::compute_threshold(){
	if(!m_thresholdComputed){
		m_thresholdComputed = true;
		ticks begin = getticks();
		boost::this_thread::sleep(boost::posix_time::millisec(1000 / m_frequency));
		ticks end = getticks();
		m_threshold = elapsed(end, begin);
	}
}
