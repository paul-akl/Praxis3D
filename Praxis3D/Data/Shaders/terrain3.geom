#version 410 core

/*uniform mat4 MVP;
//uniform mat3 NormalMatrix;

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec3 tePosition[3];
in vec3 tePatchDistance[3];

out vec3 gFacetNormal;
out vec3 gPatchDistance;
out vec3 gTriDistance;

void main()
{
    vec3 A = tePosition[2] - tePosition[0];
    vec3 B = tePosition[1] - tePosition[0];
    gFacetNormal = (MVP * vec4(normalize(cross(A, B)), 1.0)).xyz;
    
    gPatchDistance = tePatchDistance[0];
    gTriDistance = vec3(1, 0, 0);
    gl_Position = gl_in[0].gl_Position; EmitVertex();

    gPatchDistance = tePatchDistance[1];
    gTriDistance = vec3(0, 1, 0);
    gl_Position = gl_in[1].gl_Position; EmitVertex();

    gPatchDistance = tePatchDistance[2];
    gTriDistance = vec3(0, 0, 1);
    gl_Position = gl_in[2].gl_Position; EmitVertex();

    EndPrimitive();
}*/

layout(triangles) in;
//layout (triangle_strip) out;
//layout (max_vertices = 3) out;
layout(triangle_strip, max_vertices = 3) out;
//layout(line_strip, max_vertices=6) out;

in vec3 worldPos_TE[];
in vec2 texCoord_TE[];
in vec3 normal_TE[];
in vec3 eyeDir_TE[];
in mat3 TBN_TE[];
in vec3 tangent_TE[];
in vec3 bitangent_TE[];

out vec3 worldPos;
out vec2 texCoord;
out vec3 normal;
out vec3 eyeDir;
out mat3 TBN;
out vec3 tangent;
out vec3 bitangent;

uniform mat4 MVP;
uniform mat4 modelMat;
uniform mat4 modelViewMat;

void main(void)
{
	vec3 n = normalize(-cross(worldPos_TE[1].xyz - worldPos_TE[0].xyz, worldPos_TE[2].xyz - worldPos_TE[0].xyz));
	//vec3 n = cross(gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz, gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz);
	
    int i;
    for (i = 0; i < gl_in.length(); i++)
    {
		worldPos = worldPos_TE[i];
		texCoord = texCoord_TE[i];
		normal = normal_TE[i];
		normal = n;
		eyeDir = eyeDir_TE[i];
		TBN = TBN_TE[i];
		//tangent = tangent_TE[i];
		//bitangent = bitangent_TE[i];
		
		vec3 t; 
		vec3 b; 
		vec3 c1 = cross(n, vec3(0.0, 0.0, 1.0)); 
		vec3 c2 = cross(n, vec3(0.0, 1.0, 0.0)); 
		if (length(c1) > length(c2))
			t = c1;
		else
			t = c2;
		t = normalize(t);
		b = normalize(cross(n, t));
		
		tangent = t;
		bitangent = b;
	
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
		
       /* vec3 P = gl_in[i].gl_Position.xyz;
        vec3 N = normal_TE[i].xyz;
        
        gl_Position = MVP * vec4(P, 1.0);
        EmitVertex();
        
        gl_Position = MVP * vec4(P + N * 10.0, 1.0);
        EmitVertex();
		EndPrimitive();*/
    }
    EndPrimitive();
}