// Copyright (c) 1994 Darren Erik Vengroff
//
// File: mmb_stream_test.cpp
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/17/94
//



static char mmb_stream_test_id[] = "$Id: mmb_stream_test.cpp,v 1.1 1994-05-18 18:46:56 dev Exp $";

#define TPL_LOGGING	1

#include <mmb_stream.h>

#include <tpie_log.h>

int main(int argc, char **argv)
{
    int ii;
    int *ri;

    init_tpie_logs();

    {
	mmapb_stream<int> mmbw("/tmp/BTE_FS_S0", BTE_WRITE_STREAM);

	for (ii = 10000; ii--; ) {
	    mmbw.write_item(ii);
	}
    }

    {
	mmapb_stream<int> mmbw("/tmp/BTE_FS_S0", BTE_READ_STREAM);

	for (ii = 10000; ii--; ) {
	    mmbw.read_item(&ri);
	    tp_assert(*ri == ii, "Read something wierd");
	}
    }

    return 0;
}









