bit_matrix A(n,n);
bit_matrix c(n,1);

{
    unsigned int ii,jj;
    
    for (ii = n; ii--; ) {
	c[ii][0] = 0;
	for (jj = n; jj--; ) {
	    A[n-1-ii][jj] = (ii == jj);
	}
    }
}

AMI_bit_perm_object bpo(A,c);

ae = AMI_BMMC_permute(&amis0, &amis1, (AMI_bit_perm_object *)&bpo);

