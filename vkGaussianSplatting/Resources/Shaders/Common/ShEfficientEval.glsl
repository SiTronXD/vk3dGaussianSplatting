void rotVec(vec3 inVec, inout float outX, inout float outY, inout float outZ)
{
	outX = inVec.z;
	outY = inVec.x;
	outZ = inVec.y;
}

// Efficient SH basis function evaluation for the first 9 coefficients
// Based on "Efficient Spherical Harmonic Evaluation" by Peter-Pike Sloan
// https://www.ppsloan.org/publications/SHJCGT.pdf
void SHEval3(const vec3 f, inout float pSH[9])
{
	float fX, fY, fZ;
	rotVec(f, fX, fY, fZ);
	float fC0,fC1,fS0,fS1,fTmpA,fTmpB,fTmpC;
	float fZ2 = fZ*fZ;

	pSH[0] = 0.2820947917738781f;
	pSH[2] = 0.4886025119029199f*fZ;
	pSH[6] = 0.9461746957575601f*fZ2 + -0.31539156525252f;
	fC0 = fX;
	fS0 = fY;

	fTmpA = -0.48860251190292f;
	pSH[3] = fTmpA*fC0;
	pSH[1] = fTmpA*fS0;
	fTmpB = -1.092548430592079f*fZ;
	pSH[7] = fTmpB*fC0;
	pSH[5] = fTmpB*fS0;
	fC1 = fX*fC0 - fY*fS0;
	fS1 = fX*fS0 + fY*fC0;

	fTmpC = 0.5462742152960395f;
	pSH[8] = fTmpC*fC1;
	pSH[4] = fTmpC*fS1;
}

// Efficient SH basis function evaluation for the first 25 coefficients
// Based on "Efficient Spherical Harmonic Evaluation" by Peter-Pike Sloan
// https://www.ppsloan.org/publications/SHJCGT.pdf
void SHEval5(const vec3 f, inout float pSH[25])
{
	float fX, fY, fZ;
	rotVec(f, fX, fY, fZ);
	float fC0, fC1, fS0, fS1, fTmpA, fTmpB, fTmpC;
	float fZ2 = fZ*fZ;

	pSH[0] = 0.2820947917738781f;
	pSH[2] = 0.4886025119029199f*fZ;
	pSH[6] = 0.9461746957575601f*fZ2 + -0.31539156525252f;
	pSH[12] = fZ*(1.865881662950577f*fZ2 + -1.119528997770346f);
	pSH[20] = 1.984313483298443f*fZ*pSH[12] + -1.006230589874905f*pSH[6];
	fC0 = fX;
	fS0 = fY;

	fTmpA = -0.48860251190292f;
	pSH[3] = fTmpA*fC0;
	pSH[1] = fTmpA*fS0;
	fTmpB = -1.092548430592079f*fZ;
	pSH[7] = fTmpB*fC0;
	pSH[5] = fTmpB*fS0;
	fTmpC = -2.285228997322329f*fZ2 + 0.4570457994644658f;
	pSH[13] = fTmpC*fC0;
	pSH[11] = fTmpC*fS0;
	fTmpA = fZ*(-4.683325804901025f*fZ2 + 2.007139630671868f);
	pSH[21] = fTmpA*fC0;
	pSH[19] = fTmpA*fS0;
	fC1 = fX*fC0 - fY*fS0;
	fS1 = fX*fS0 + fY*fC0;

	fTmpA = 0.5462742152960395f;
	pSH[8] = fTmpA*fC1;
	pSH[4] = fTmpA*fS1;
	fTmpB = 1.445305721320277f*fZ;
	pSH[14] = fTmpB*fC1;
	pSH[10] = fTmpB*fS1;
	fTmpC = 3.31161143515146f*fZ2 + -0.47308734787878f;
	pSH[22] = fTmpC*fC1;
	pSH[18] = fTmpC*fS1;
	fC0 = fX*fC1 - fY*fS1;
	fS0 = fX*fS1 + fY*fC1;

	fTmpA = -0.5900435899266435f;
	pSH[15] = fTmpA*fC0;
	pSH[9] = fTmpA*fS0;
	fTmpB = -1.770130769779931f*fZ;
	pSH[23] = fTmpB*fC0;
	pSH[17] = fTmpB*fS0;
	fC1 = fX*fC0 - fY*fS0;
	fS1 = fX*fS0 + fY*fC0;

	fTmpC = 0.6258357354491763f;
	pSH[24] = fTmpC*fC1;
	pSH[16] = fTmpC*fS1;
}