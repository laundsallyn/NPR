#ifndef SKINNING_GUI_H
#define SKINNING_GUI_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <GL/glut.h>
#include <AntTweakBar.h>
#include "bone_geometry.h"

/*
 * Hint: call glUniformMatrix4fv on thest pointers
 */
struct MatrixPointers {
	const float *projection, *model, *view;
};

class GUI {
public:
	GUI(GLFWwindow*);
	~GUI();
	void assignMesh(Mesh*);
	TwBar* getTweakBar(){
		return tBar_;
	}
	void keyCallback(int key, int scancode, int action, int mods);
	void mousePosCallback(double mouse_x, double mouse_y);
	void mouseButtonCallback(int button, int action, int mods);
	void updateMatrices();
	MatrixPointers getMatrixPointers() const;

	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void MousePosCallback(GLFWwindow* window, double mouse_x, double mouse_y);
	static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

	glm::vec3 getCenter() const { return center_; }
	const glm::vec3& getCamera() const { return eye_; }
	bool isPoseDirty() const { return pose_changed_; }
	void clearPose() { pose_changed_ = false; }
	const float* getLightPositionPtr() const { return &light_position_[0]; }
	
	int getCurrentBone() const { return current_bone_; }
	bool setCurrentBone(int i);

	bool isTransparent() const { return transparent_; }
	void setControl(int index, float f){
		mesh_->control[index] = f;
	}
	float getControl(int index){
		return mesh_->control[index];
	}
	bool isOutlineShow() { return show_outline; }
	bool isNPRcolor() { return show_npr_color; }
	float getBGColor(int index) { return bg_color[index]; }
	glm::vec3 getOutlineColor() { return outline_color; }
	int getOutlineSize() { return outline_size; }
	void setGarlicParam(glm::vec4 gar);
	void setDefaultOutline() {
		bg_color = glm::vec3(1.0f, 1.0f, 1.0f);
		outline_color = glm::vec3(0.0f, 0.0f, 0.0f);
		outline_size = 900;
	}
	bool showFloor() { return show_floor; }
private:
	GLFWwindow* window_;
	Mesh* mesh_;
	TwBar* tBar_;
	int window_width_, window_height_;
	bool change_garlic_param = false;
	bool show_outline = false;
	bool show_npr_color = false;
	bool show_floor = true;
	glm::vec3 bg_color;
	glm::vec3 outline_color;
	unsigned int outline_size = 900;
	bool drag_state_ = false;
	bool fps_mode_ = false;
	bool pose_changed_ = true;
	bool transparent_ = false;
	int current_bone_ = -1;
	int current_button_ = -1;
	float roll_speed_ = 0.1;
	float last_x_ = 0.0f, last_y_ = 0.0f, current_x_ = 0.0f, current_y_ = 0.0f;
	float camera_distance_ = 100.0;
	float pan_speed_ = 0.1f;
	float rotation_speed_ = 0.02f;
	float zoom_speed_ = 0.1f;
	float aspect_;

	glm::vec3 eye_ = glm::vec3(0.0f, 0.1f, camera_distance_);
	glm::vec3 up_ = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 look_ = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 tangent_ = glm::cross(look_, up_);
	glm::vec3 center_ = eye_ - camera_distance_ * look_;
	glm::mat3 orientation_ = glm::mat3(tangent_, up_, look_);
	glm::vec4 light_position_;

	glm::mat4 view_matrix_ = glm::lookAt(eye_, center_, up_);
	glm::mat4 projection_matrix_;
	glm::mat4 model_matrix_ = glm::mat4(1.0f);

	bool captureWASDUPDOWN(int key, int action);

};

#endif
