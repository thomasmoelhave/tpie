/*
 * Copyright (c) 1994 Darren Erik Vengroff
 *
 * File: int_cmp.c
 * Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
 * Created: 8/30/94
 */



static char int_cmp_id[] = "$Id: int_cmp.c,v 1.1 1994-08-31 19:58:10 darrenv Exp $";


int int_cmp(const void *p1, const void *p2)
{
    return *((int *)p1) - *((int *)p2);
}



