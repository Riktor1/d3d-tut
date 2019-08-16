struct VSOut {
	float3 color : COLOR;
	float4 pos : SV_POSITION;
};

cbuffer Cbuf {
	matrix transform; //using row major here because GPU needs to know since it is column_major, but this calling is slower
};

VSOut main(float3 color : COLOR, float4 pos : POSITION /*input semantic*/ ){
	
	VSOut vso; //create output object
	vso.pos = mul(float4(pos.x, pos.y, 0.0f, 1.0f), transform); //set the position of struct object [mul(,) takes the vertex and multiplies it by the transform matrix
																//to create the roation transform
	vso.color = color; //set the colorl of struct object
	return vso; //return the output object
}