/*
 * Copyright (c) 1994 Darren Erik Vengroff
 *
 * File: int_cmp.c
 * Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
 * Created: 8/30/94
 */



static char int_cmp_id[] = "$Id: int_cmp.c,v 1.2 1994-10-07 15:41:09 darrenv Exp $";


int c_int_cmp(const void *p1, const void *p2)
{
    return *((int *)p1) - *((int *)p2);
}



