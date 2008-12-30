#ifndef _TPIE_AMI_RECTANGLE_COMPARATORS_H
#define _TPIE_AMI_RECTANGLE_COMPARATORS_H

namespace tpie {

    namespace ami {

	////////////////////////////////////////////////////////////////////////////
        /// Comparator for sorting by lexicographical x-order.
	////////////////////////////////////////////////////////////////////////////
	template<class coord_t>
	struct sort_boxes_along_x_axis {

	    ////////////////////////////////////////////////////////////////////////
	    /// Sort by x-coordinate of left boundary, use x-coordinate of right
            /// boundary to break ties.
            ////////////////////////////////////////////////////////////////////////
	    inline bool operator()(const rectangle<coord_t, bid_t>& t1, 
				   const rectangle<coord_t, bid_t>& t2) const {
		if (t1.get_left() == t2.get_left()) {
		    return (t1.get_right() < t2.get_right());
		}
		else {
		    return (t1.get_left() < t2.get_left());
		}
	    }
	};
	
    }  //  ami namespace

}  //  tpie namespace

namespace tpie {

    namespace ami {

	////////////////////////////////////////////////////////////////////////////
        ///  Comparator for sorting by lexicographical y-order.
	////////////////////////////////////////////////////////////////////////////
	template<class coord_t>
	struct sort_boxes_along_y_axis {

	    ////////////////////////////////////////////////////////////////////////
	    /// Sort by y-coordinate of lower boundary, use y-coordinate of upper
            /// boundary to break ties.
            ////////////////////////////////////////////////////////////////////////
	    inline bool operator()(const rectangle<coord_t, bid_t>& t1, 
				   const rectangle<coord_t, bid_t>& t2) const {
		if (t1.get_lower() == t2.get_lower()) {
		    return (t1.get_upper() < t2.get_upper());
		}
		else {
		    return (t1.get_lower() < t2.get_lower());
		}
	    }
	};

    }  //  ami namespace

}  //  tpie namespace


namespace tpie {

    namespace ami {

	////////////////////////////////////////////////////////////////////////////
        /// Comparator for sorting by a secondary key.
        /// Used in forced reinsertion with distance to center as the key (hence 
        /// the name).
	////////////////////////////////////////////////////////////////////////////
	template<class coord_t>
	struct sort_by_center_distance {

	    ////////////////////////////////////////////////////////////////////////
	    /// Sort by the second component of the pair.
	    ////////////////////////////////////////////////////////////////////////
	    bool operator()(const std::pair<TPIE_OS_SIZE_T, coord_t>& t1, 
			    const std::pair<TPIE_OS_SIZE_T, coord_t>& t2) {
		return (t1. second < t2.second);
	    }
	};

    }  //  ami namespace

}  //  tpie namespace


#endif
