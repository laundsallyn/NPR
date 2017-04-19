#include "procedure_geometry.h"
#include "bone_geometry.h"
#include "config.h"
#include <glm/gtx/transform.hpp>
#include <glm/ext.hpp>
#include <iostream>

void create_floor(std::vector<glm::vec4>& floor_vertices, std::vector<glm::uvec3>& floor_faces)
{
	floor_vertices.push_back(glm::vec4(kFloorXMin, kFloorY, kFloorZMax, 1.0f));
	floor_vertices.push_back(glm::vec4(kFloorXMax, kFloorY, kFloorZMax, 1.0f));
	floor_vertices.push_back(glm::vec4(kFloorXMax, kFloorY, kFloorZMin, 1.0f));
	floor_vertices.push_back(glm::vec4(kFloorXMin, kFloorY, kFloorZMin, 1.0f));
	floor_faces.push_back(glm::uvec3(0, 1, 2));
	floor_faces.push_back(glm::uvec3(2, 3, 0));
}

// Hints: Generate a lattice in [-0.5, 0, 0] x [0.5, 1, 0] We wrap this
// around in the vertex shader to produce a very smooth cylinder.  We only
// need to send a small number of points.  Controlling the grid size gives a
// nice wireframe.

void create_linemesh(LineMesh& line_mesh, Skeleton skeleton){
	line_mesh.clear();
	for(int i = 1; i < skeleton.bones.size(); ++i){
		Bone* b = skeleton.bones[i];
		line_mesh.vertices.push_back(b->WorldPointFromBone(glm::vec4( 0.0,0.0,0.0,1)));
		line_mesh.vertices.push_back(b->WorldPointFromBone(glm::vec4(b->length, 0, 0,1)));
		line_mesh.bone_lines.push_back(glm::uvec2(line_mesh.currentIndex, line_mesh.currentIndex+1));
		line_mesh.currentIndex+= 2;
	}
}

void create_default(LineMesh& lm){
	lm.vertices.push_back(glm::vec4( 0.0, 0.0, 0.0, 1));
	lm.vertices.push_back(glm::vec4(10.0, 0.0, 0.0, 1));
	lm.bone_lines.push_back(glm::uvec2(0, 1));
	lm.vertices.push_back(glm::vec4( 0.0, 0.0, 0.0,1));
	lm.vertices.push_back(glm::vec4( 0.0,10.0, 0.0,1));
	lm.bone_lines.push_back(glm::uvec2(2, 3));
	lm.vertices.push_back(glm::vec4( 0.0, 0.0, 0.0,1));
	lm.vertices.push_back(glm::vec4( 0.0, 0.0,10.0,1));
	lm.bone_lines.push_back(glm::uvec2(4, 5));
}

void create_cylinder(LineMesh& lm, Skeleton sk, int index){
	lm.clear();

	Bone* b = sk.bones[index];
	glm::vec4 start = glm::vec4(0.0,0.0,0.0,1.0);
	glm::vec4 end =  glm::vec4(b->length, 0, 0,1);

	glm::vec3 offset = b->bd;
	offset.x =0.f; offset.y = 0.2f; offset.z = 0.2;
	float deg = 0.0;
	int lastS = -1; int lastE = -1;
	glm::vec3 axis = glm::normalize(glm::vec3(start - end));
	start += glm::vec4(offset,0);
	end += glm::vec4(offset,0);
	while(deg <= 360){
		float rad = glm::radians(30.0);
		start = glm::rotate(rad, axis) * start;
		end = glm::rotate(rad, axis) * end;
		lm.vertices.push_back( b->WorldPointFromBone(start));
		lm.vertices.push_back( b->WorldPointFromBone(end));
		lm.bone_lines.push_back(glm::uvec2(lm.currentIndex,lm.currentIndex+1));
		if(lastS > -1){
			lm.bone_lines.push_back(glm::uvec2(lastS,lm.currentIndex));
			lm.bone_lines.push_back(glm::uvec2(lastE,lm.currentIndex+1));
		}
		lastS = lm.currentIndex;
		lastE = lm.currentIndex + 1;
		deg += 30;	
		lm.currentIndex += 2;
	}
}

//remember to call create_cylinder and create_bone_coordinate at the same time.

void create_coordinate(LineMesh& lm, Skeleton sk, int index){
		lm.clear();
		Bone* b = sk.bones[index];

		glm::vec4 start = b->WorldPointFromBone(glm::vec4(0,0,0,1));
		glm::vec4 normal = b->WorldPointFromBone(glm::vec4(0,0.5, 0,1));
		glm::vec4 binorm = b->WorldPointFromBone(glm::vec4(0,0,.5,1));
	
		lm.vertices.push_back(start);
		lm.vertices.push_back(normal);
		lm.vertices.push_back(binorm);
		lm.bone_lines.push_back(glm::uvec2(0,1));
		lm.bone_lines.push_back(glm::uvec2(0,2));
		lm.color.push_back(glm::vec4(0,0,0,1));
		lm.color.push_back(glm::vec4(1,0,0,1));
		lm.color.push_back(glm::vec4(0,0,1,1));
}

