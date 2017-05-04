xof 0303txt 0032
template Vector {
 <3d82ab5e-62da-11cf-ab39-0020af71e433>
 FLOAT x;
 FLOAT y;
 FLOAT z;
}

template MeshFace {
 <3d82ab5f-62da-11cf-ab39-0020af71e433>
 DWORD nFaceVertexIndices;
 array DWORD faceVertexIndices[nFaceVertexIndices];
}

template Mesh {
 <3d82ab44-62da-11cf-ab39-0020af71e433>
 DWORD nVertices;
 array Vector vertices[nVertices];
 DWORD nFaces;
 array MeshFace faces[nFaces];
 [...]
}

template MeshNormals {
 <f6f23f43-7686-11cf-8f52-0040333594a3>
 DWORD nNormals;
 array Vector normals[nNormals];
 DWORD nFaceNormals;
 array MeshFace faceNormals[nFaceNormals];
}

template Coords2d {
 <f6f23f44-7686-11cf-8f52-0040333594a3>
 FLOAT u;
 FLOAT v;
}

template MeshTextureCoords {
 <f6f23f40-7686-11cf-8f52-0040333594a3>
 DWORD nTextureCoords;
 array Coords2d textureCoords[nTextureCoords];
}

template ColorRGBA {
 <35ff44e0-6c7c-11cf-8f52-0040333594a3>
 FLOAT red;
 FLOAT green;
 FLOAT blue;
 FLOAT alpha;
}

template ColorRGB {
 <d3e16e81-7835-11cf-8f52-0040333594a3>
 FLOAT red;
 FLOAT green;
 FLOAT blue;
}

template Material {
 <3d82ab4d-62da-11cf-ab39-0020af71e433>
 ColorRGBA faceColor;
 FLOAT power;
 ColorRGB specularColor;
 ColorRGB emissiveColor;
 [...]
}

template MeshMaterialList {
 <f6f23f42-7686-11cf-8f52-0040333594a3>
 DWORD nMaterials;
 DWORD nFaceIndexes;
 array DWORD faceIndexes[nFaceIndexes];
 [Material <3d82ab4d-62da-11cf-ab39-0020af71e433>]
}

template TextureFilename {
 <a42790e1-7810-11cf-8f52-0040333594a3>
 STRING filename;
}


Mesh {
 16;
 0.356930;1.073200;-0.874630;,
 0.277500;1.073200;-0.874630;,
 0.277500;0.880010;-0.822870;,
 0.356930;0.880010;-0.822870;,
 0.594140;0.880010;-0.822870;,
 0.677500;0.880010;-0.822870;,
 0.677500;1.073200;-0.874630;,
 0.593660;1.073200;-0.874630;,
 0.448370;0.970910;-0.847220;,
 0.448340;0.982590;-0.850350;,
 0.402820;0.982350;-0.850290;,
 0.402820;0.970910;-0.847220;,
 0.476170;0.963330;-0.845190;,
 0.414180;0.963330;-0.845190;,
 0.414240;0.947710;-0.841010;,
 0.476200;0.947710;-0.841010;;
 8;
 3;0,1,2;,
 3;0,2,3;,
 3;4,5,6;,
 3;4,6,7;,
 3;8,9,10;,
 3;8,10,11;,
 3;12,13,14;,
 3;12,14,15;;

 MeshNormals {
  16;
  0.000000;0.258795;0.965932;,
  0.000000;0.258795;0.965932;,
  0.000000;0.258795;0.965932;,
  0.000000;0.258795;0.965932;,
  0.000000;0.258795;0.965932;,
  0.000000;0.258795;0.965932;,
  0.000000;0.258795;0.965932;,
  0.000000;0.258795;0.965932;,
  -0.000076;0.258902;0.965904;,
  -0.000091;0.258847;0.965918;,
  -0.000015;0.259130;0.965842;,
  0.000000;0.259186;0.965827;,
  0.000000;0.258510;0.966009;,
  0.000000;0.258510;0.966009;,
  0.000000;0.258510;0.966009;,
  0.000000;0.258510;0.966009;;
  8;
  3;0,1,2;,
  3;0,2,3;,
  3;4,5,6;,
  3;4,6,7;,
  3;8,9,10;,
  3;8,10,11;,
  3;12,13,14;,
  3;12,14,15;;
 }

 MeshTextureCoords {
  16;
  0.801420;0.000000;,
  1.000000;0.000000;,
  1.000000;1.000000;,
  0.801420;1.000000;,
  0.208410;1.000000;,
  0.000000;1.000000;,
  0.000000;0.000000;,
  0.209610;0.000000;,
  0.572810;0.529500;,
  0.572900;0.469040;,
  0.686700;0.470290;,
  0.686700;0.529510;,
  0.503336;0.568704;,
  0.658303;0.568705;,
  0.658163;0.649553;,
  0.503262;0.649553;;
 }

 MeshMaterialList {
  1;
  8;
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0;

  Material {
   0.800000;0.800000;0.800000;1.000000;;
   5.000000;
   0.000000;0.000000;0.000000;;
   0.500000;0.500000;0.500000;;

   TextureFilename {
    "meterpanel_side.png";
   }
  }
 }
}