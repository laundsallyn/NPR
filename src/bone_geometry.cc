#include "config.h"
#include "bone_geometry.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <glm/gtx/io.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/ext.hpp>

#include <vector>

/*
 * For debugging purpose.
 */
template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v) {
	size_t count = std::min(v.size(), static_cast<size_t>(10));
	for (size_t i = 0; i < count; ++i) os << i << " " << v[i] << "\n";
	os << "size = " << v.size() << "\n";
	return os;
}

std::ostream& operator<<(std::ostream& os, const BoundingBox& bounds)
{
	os << "min = " << bounds.min << " max = " << bounds.max;
	return os;
}



// FIXME: Implement bone animation.


Mesh::Mesh()
{
}

Mesh::~Mesh()
{
}

void printMat(glm::mat4 mat);

void Mesh::loadpmd(const std::string& fn)
{
	MMDReader mr;
	mr.open(fn);
	mr.getMesh(vertices, faces, vertex_normals, uv_coordinates);
	computeBounds();
	mr.getMaterial(materials);

	// FIXME: load skeleton and blend weights from PMD file
	//        also initialize the skeleton as needed
	int id = 0;
	glm::vec3 offset;
	int parent;

	while (mr.getJoint(id, offset, parent)) {
		//create Joints with data
		Joint j(id, offset, parent);
		skeleton.joints.push_back(j);
		++id;
	}
	std::vector<SparseTuple> weights;
	mr.getJointWeights(weights);
	std::cout << "Number of Joints found: " << id << std::endl;
	std::cout << "Joint List size: " << skeleton.joints.size() << std::endl;
	std::cout << "Weights List size: " << weights.size() << std::endl;

	// If joint.parent == -1, that joint is cannot represent a bone
	// bone is based off end joint -> 

	skeleton.bones.resize(skeleton.joints.size());

	skeleton.bones[0] = nullptr; // there is no bone 0, because joint 0 
	                             // is not an endpoint for a bone
	for (uint n = 1; n < skeleton.joints.size(); ++n) {
		skeleton.constructBone(n);
	}

	weight_map.resize(getNumberOfBones()+1);
	for (uint n = 0; n < weight_map.size(); ++n) {
		weight_map[n].resize(vertices.size());
	}
	for (uint n = 0; n < weights.size(); ++n) {
		SparseTuple w = weights[n];
		Joint p = skeleton.joints[w.jid];
		for (uint m = 0; m < p.children.size(); ++m) {
			int bone_id = p.children[m];
			weight_map[bone_id][w.vid] = w.weight;
		}
	}


	// for (int n = 1; n < 4; ++n) {
	// 	Bone* b = skeleton.bones[n];
	// 	std::cout << "-- Bone " << b->id << " --" << std::endl;
	// 	// std::cout << glm::to_string(b->WorldPointFromBone(glm::vec4(0, 0 ,0, 1))) << std::endl;
	// 	// std::cout << glm::to_string(b->WorldPointFromBone(glm::vec4(b->length, 0 ,0, 1))) << std::endl;
	// 	// std::cout << glm::to_string(b->WorldPointFromBone(glm::vec4(1, 1, 1, 1))) << std::endl;
	// }
}

void Mesh::updateAnimation()
{
	animated_vertices.resize(vertices.size());
	// FIXME: blend the vertices to animated_vertices, rather than copy
	//        the data directly.
	std::vector<glm::mat4> blend;
	for (uint i = 1; i < skeleton.bones.size(); ++i) {
		Bone *b = getBone(i);
		glm::mat4 u = b->UndeformedToWorld();
		u = glm::inverse(u);
		glm::mat4 d = b->DeformedToWorld();
		blend.push_back(d * u);
	}
	for (uint i = 0; i < vertices.size(); ++i) {
		glm::mat4 t = glm::mat4(0.f);
		for(uint j = 1; j < skeleton.bones.size(); ++j){
			float w = weight_map[j][i];
			if (w == 0.0) 
				continue;
			t += (w * blend[j-1]);
		}
		animated_vertices[i] = t * vertices[i]; 

	}
}


void Mesh::computeBounds()
{
	bounds.min = glm::vec3(std::numeric_limits<float>::max());
	bounds.max = glm::vec3(-std::numeric_limits<float>::max());
	for (const auto& vert : vertices) {
		bounds.min = glm::min(glm::vec3(vert), bounds.min);
		bounds.max = glm::max(glm::vec3(vert), bounds.max);
	}
}

void Skeleton::constructBone(int jid) {
// std::cout << "Skeleton::constructBone: joint.id = " << j.id << std::endl;
	if (jid <= 0 || bones[jid] != nullptr) {
		return;
	}
	Joint j = joints[jid];

	constructBone(j.parent);
	Bone *b = new Bone(joints[j.parent], j);
	bones[j.id] = b;
	joints[j.parent].children.push_back(j.id);
	if (j.parent > 0) {
		b->parent = bones[j.parent];
	} else {
		b->parent = nullptr;
	}
	return;
}

Bone* Mesh::getBone(int n) {
	return skeleton.bones[n];
}

/*
 * For this bone, returns T*R of parent up to,
 * and including, this bone
 * For example, For Bone1 (no parent)
 *     returns T1*R1
 * For Bone3 (1<-2<-3)
       reutrn T1*R1*T2*R2*T3*R3
 */


void printMat(glm::mat4 mat) {
	std::cout << "glm::mat4" << std::endl;
	for (int i = 0; i < 4; ++i) {
		std::cout << glm::to_string(glm::row(mat, i)) << std::endl;
	}
	std::cout << std::endl;
}

glm::mat4 Bone::makeRotateMat(glm::vec3 offset) {

	glm::vec3 tangent = glm::normalize(offset);
	int normalInd = abs(tangent.x) < abs(tangent.y) ? 0 : 1;
	normalInd = abs(tangent[normalInd]) < abs(tangent.z) ? normalInd : 2;
	glm::vec3 normal(0,0,0);
	normal[normalInd] = 1;
	normal = glm::normalize(glm::cross(tangent, normal));

	glm::vec3 bd = glm::normalize(glm::cross(tangent, normal));
	glm::mat4 r = glm::mat4(glm::mat3(tangent, normal, bd));
	r[3][3] = 1.0f;

	return r;
}

glm::mat4& Bone::getAbsRotation() {
	return absRotation;
}

glm::mat4& Bone::getRelRotation() {
	return sRotation;
}

glm::mat4& Bone::getTranslation() {
	return translation;
}

glm::mat4& Bone::getDeformedRotation() {
	return sRotation;
}

glm::mat4& Bone::getUndeformedRotation() {
	return relRotation;
}

glm::mat4 Bone::getWorldMat() {
	if (parent == nullptr) {
		return translation * getDeformedRotation();
	} else {
		return (parent->getWorldMat() * translation * getDeformedRotation());
	}
}

glm::mat4 Bone::BoneToWorldRotation() {
	if (parent == nullptr) {
		return getDeformedRotation();
	} else {
		return parent->getDeformedRotation() * getDeformedRotation();
	}
}

// returns the world coordinate for a point
// that is relative to a bone
// e.g. (0,0,0) for a bone
// would return the bone's start point.
glm::vec4 Bone::WorldPointFromBone(glm::vec4 p) {
	return getWorldMat() * p;
}

glm::mat4 Bone::UndeformedToWorld() {
	if (parent == nullptr) {
		return translation * getUndeformedRotation();
	} else {
		return (parent->UndeformedToWorld() * translation * getUndeformedRotation());
	}
}

glm::mat4 Bone::DeformedToWorld() {
	if (parent == nullptr) {
		return translation * getDeformedRotation();
	} else {
		return (parent->DeformedToWorld() * translation * getDeformedRotation());
	}
}