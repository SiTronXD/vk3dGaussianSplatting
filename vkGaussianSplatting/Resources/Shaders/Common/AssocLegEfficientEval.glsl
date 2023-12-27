
float assocLeg00(float x)
{
	return 1.0f;
}

float assocLeg01(float x)
{
	return x;
}

float assocLeg02(float x)
{
	return 0.5f * (3.0f * x * x - 1.0f);
}

float assocLeg03(float x)
{
	return 0.5f * x * (5.0f * x * x - 3.0f);
}

float assocLeg04(float x)
{
	return 0.125f * (35.0f * x * x * x * x - 30.0f * x * x + 3.0f);
}

float assocLeg05(float x)
{
	return 0.125f * x * (63.0f * x * x * x * x - 70.0f * x * x + 15.0f);
}

// Efficient evaluation of associated legendre polynomials.
// Assumption: m = 0, l is within [0, 5]
float evalAssocLeg(int l, float x)
{
	if(l == 0) return assocLeg00(x);
	else if(l == 1) return assocLeg01(x);
	else if(l == 2) return assocLeg02(x);
	else if(l == 3) return assocLeg03(x);
	else if(l == 4) return assocLeg04(x);
	else if(l == 5) return assocLeg05(x);

	return 0.0f;
}