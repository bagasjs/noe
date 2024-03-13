#version 330 core

layout (location=0) out vec4 o_FragColor;

in vec4 v_Color;
in vec2 v_TexCoords;
in float v_TextureIndex;

uniform sampler2D u_Textures[8];

void main() {
    switch(int(v_TextureIndex)) {
        case 0: 
            o_FragColor = texture(u_Textures[0], v_TexCoords.xy) * v_Color;
            break;
        case 1: 
            o_FragColor = texture(u_Textures[1], v_TexCoords.xy) * v_Color;
            break;
        case 2: 
            o_FragColor = texture(u_Textures[2], v_TexCoords.xy) * v_Color;
            break;
        case 3: 
            o_FragColor = texture(u_Textures[3], v_TexCoords.xy) * v_Color;
            break;
        case 4: 
            o_FragColor = texture(u_Textures[4], v_TexCoords.xy) * v_Color;
            break;
        case 5: 
            o_FragColor = texture(u_Textures[5], v_TexCoords.xy) * v_Color;
            break;
        case 6: 
            o_FragColor = texture(u_Textures[6], v_TexCoords.xy) * v_Color;
            break;
        case 7: 
            o_FragColor = texture(u_Textures[7], v_TexCoords.xy) * v_Color;
            break;
        default:
            o_FragColor = v_Color;
            break;
    }
}
