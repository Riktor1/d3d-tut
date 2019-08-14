struct VSOut {
	float3 color : COLOR;
	float4 pos : SV_POSITION;
};

VSOut main(float3 color : COLOR, float4 pos : POSITION /*input semantic*/ ){
	
	VSOut vso; //create output object
	vso.pos = float4(pos.x, pos.y, 0.0f, 1.0f); //set the position of struct object
	vso.color = color; //set the colorl of struct object
	return vso; //return the output object
}