#include "gui.h"
#include "config.h"
#include <jpegio.h>
#include "bone_geometry.h"
#include "procedure_geometry.h"
#include <iostream>
#include <debuggl.h>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/ext.hpp>


namespace {
	bool intersectBody(const glm::vec3& origin, const glm::vec3& direction,
		float radius, float height, float* t) {
		//TODO: check for intersection on body of a cylinder
		float RAY_EPSILON = 0.000001;
		float px = origin[0];
		float pz = origin[2];
		float dx = direction[0];
		float dz = direction[2];

		float a = dx*dx + dz*dz;          // x^2 + z^2 = 1
		float b = 2.0*(px*dx + pz*dz);    // ?
		float c = px*px + pz*pz - radius*radius; // x^2 + z^2 - r^2 = c

		if (a == 0.0) {
			// direction is parallel to cylinder
			// cannot intersect with body
			return false;
		}

		float discriminant = b*b - 4.0*a*c; //b^2 - 4ac
		if (discriminant < 0.0) {
			// discriminant is imaginary
			return false;
		}
		discriminant = sqrt(discriminant);

		// -b +/- sqrt(b^2 - 4ac)/2a
		float t2 = (-b + discriminant) / (2.0 *a);
		if (t2 <= RAY_EPSILON) {
			return false; //why?
		}

		float t1 = (-b - discriminant) / (2.0 *a);
		if (t1 > RAY_EPSILON) {
			glm::vec3 p = origin + direction * t1;
			float y = p[1];
			if (y >= 0.0 && y <= height) {
				*t = t1;
				return true;
			}
		}

		glm::vec3 p = origin + direction * t2;
		float y = p[1];
		if (y >= 0.0 && y <= height) {
			*t = t2;
			return true;
		}
		return false;
	}

	bool intersectCaps(const glm::vec3& origin, const glm::vec3& direction,
		float radius, float height, float* t) {
		//TODO: check for intersection on caps of cylinder
		float py = origin[1];
		float dy = direction[1];
		float RAY_EPSILON = 0.000001;

		if (dy == 0.0) { // parallel to caps
			return false;
		}

		float t1, t2;

		if (dy > 0.0) {
			t1 = (-py)/dy;
			t2 = (height-py)/dy;
		} else {
			t1 = (height-py)/dy;
			t2 = (-py)/dy;
		}

		if (t2 < RAY_EPSILON) {
			return false;
		}

		if (t1 >= RAY_EPSILON) {
			glm::vec3 p = origin + direction*t1;
			if ((p[0]*p[0] + p[2]*p[2]) <= radius*radius) {
				*t = t1;
				return true;
			} 
		}

		glm::vec3 p = origin + direction*t2;
		if ((p[0]*p[0] + p[2]*p[2]) <= radius*radius) {
			*t = t2;
			return true;
		}

		return false;
	}

	// Intersect a cylinder with radius 1/2, height 1, with base centered at
	// (0, 0, 0) and up direction (0, 1, 0).
	bool IntersectCylinder(const glm::vec3& origin, const glm::vec3& direction,
			float radius, float height, float* t)
	{

		if (intersectCaps(origin, direction, radius, height, t)) {
			float tt;
			if (intersectBody(origin, direction, radius, height, &tt)) {
				if (tt < (*t)) {
					*t = tt;
				}
			}
			return true;
		} else {
			return intersectBody(origin, direction, radius, height, t);
		}
	}
}

GUI::GUI(GLFWwindow* window)
	:window_(window)
{
	glfwSetWindowUserPointer(window_, this);
	glfwSetKeyCallback(window_, KeyCallback);
	glfwSetCursorPosCallback(window_, MousePosCallback);
	glfwSetMouseButtonCallback(window_, MouseButtonCallback);

	glfwGetWindowSize(window_, &window_width_, &window_height_);
	float aspect_ = static_cast<float>(window_width_) / window_height_;
	projection_matrix_ = glm::perspective((float)(kFov * (M_PI / 180.0f)), aspect_, kNear, kFar);
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(window_width_, window_height_);
	tBar_ = TwNewBar("NameOfTweakBar");
}

GUI::~GUI()
{
}

void GUI::assignMesh(Mesh* mesh)
{
	mesh_ = mesh;
	center_ = mesh_->getCenter();
}

void GUI::keyCallback(int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window_, GL_TRUE);
		return ;
	}
	if (key == GLFW_KEY_J && action == GLFW_RELEASE) {
		GLubyte *pixels = (GLubyte*) malloc(4 * window_width_ * window_height_);
		glReadPixels(0, 0, window_width_, window_height_, GL_RGB, GL_UNSIGNED_BYTE, pixels);
		SaveJPEG("out.jpg", window_width_,window_height_, pixels);
		free(pixels);
	}

	if (captureWASDUPDOWN(key, action))
		return ;
	if (key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT) {
		float roll_speed;
		if (key == GLFW_KEY_RIGHT)
			roll_speed = -roll_speed_;
		else
			roll_speed = roll_speed_;
		// FIXME: actually roll the bone here
		if(current_bone_ == -1)
			return;
		Bone *b = mesh_->getBone(current_bone_);
		glm::mat4 rotated = glm::rotate(roll_speed, glm::vec3(b->sRotation[0]));
		b->sRotation = rotated * b->sRotation;
		pose_changed_ = true;

	} else if (key == GLFW_KEY_C && action != GLFW_RELEASE) {
		fps_mode_ = !fps_mode_;
	} else if (key == GLFW_KEY_LEFT_BRACKET && action == GLFW_RELEASE) {

		current_bone_--;
		current_bone_ += mesh_->getNumberOfBones();
		current_bone_ %= mesh_->getNumberOfBones();
		if(current_bone_ - 1 < 1)
			current_bone_ = mesh_->getNumberOfBones();

	} else if (key == GLFW_KEY_RIGHT_BRACKET && action == GLFW_RELEASE) {
		current_bone_++;
		current_bone_ += mesh_->getNumberOfBones();
		current_bone_ %= mesh_->getNumberOfBones();
		if( current_bone_ == 0 or current_bone_ > mesh_->getNumberOfBones()+1)
			current_bone_ = 1;
	} else if (key == GLFW_KEY_T && action != GLFW_RELEASE) {
		transparent_ = !transparent_;
	} else if (key == GLFW_KEY_R && action != GLFW_RELEASE){
		// randomize garlic_parameter value
		for(int i = 0; i < 4; ++i){
			float r = static_cast<float>(rand())/ static_cast<float> (RAND_MAX);
			mesh_->garlic_param[i] = r;	
		}
		std::cout<<glm::to_string(mesh_->garlic_param)<<std::endl;
		
	} else if (key == GLFW_KEY_1 && action != GLFW_RELEASE) {
		mesh_->control[0] = 0;
		mesh_->control[1] = 0;
		mesh_->control[2] = 0;
	} else if (key == GLFW_KEY_2 && action != GLFW_RELEASE) {
		mesh_->control[0] = abs(mesh_->control[0] - 1);
	} else if (key == GLFW_KEY_3 && action != GLFW_RELEASE) {
		mesh_->control[1] = abs(mesh_->control[1] - 1);
		mesh_->control[2] = 0;
	} else if (key == GLFW_KEY_4 && action != GLFW_RELEASE) {
		mesh_->control[0] = 1;
		mesh_->control[1] = 0;
		mesh_->control[2] = 1;
	}
}

void GUI::mousePosCallback(double mouse_x, double mouse_y)
{
	last_x_ = current_x_;
	last_y_ = current_y_;
	current_x_ = mouse_x;
	current_y_ = window_height_ - mouse_y;
	float delta_x = current_x_ - last_x_;
	float delta_y = current_y_ - last_y_;
	if (sqrt(delta_x * delta_x + delta_y * delta_y) < 1e-15)
		return;
	glm::vec3 mouse_direction = glm::normalize(glm::vec3(delta_x, delta_y, 0.0f));
	glm::vec2 mouse_start = glm::vec2(last_x_, last_y_);
	glm::vec2 mouse_end = glm::vec2(current_x_, current_y_);
	glm::uvec4 viewport = glm::uvec4(0, 0, window_width_, window_height_);

	bool drag_camera = drag_state_ && current_button_ == GLFW_MOUSE_BUTTON_RIGHT;
	bool drag_bone = drag_state_ && current_button_ == GLFW_MOUSE_BUTTON_LEFT;

	if (drag_camera) {
		glm::vec3 axis = glm::normalize(
				orientation_ *
				glm::vec3(mouse_direction.y, -mouse_direction.x, 0.0f)
				);
		orientation_ =
			glm::mat3(glm::rotate(rotation_speed_, axis) * glm::mat4(orientation_));
		tangent_ = glm::column(orientation_, 0);
		up_ = glm::column(orientation_, 1);
		look_ = glm::column(orientation_, 2);
	} else if (drag_bone && current_bone_ != -1) {
		// FIXME: Handle bone rotation
		Bone *b = mesh_->getBone(current_bone_);
		glm::vec3 wc_start = glm::unProject(glm::vec3(mouse_start, 0), view_matrix_ * model_matrix_, projection_matrix_, viewport);
		glm::vec3 wc_end = glm::unProject(glm::vec3(mouse_end, 0), view_matrix_ * model_matrix_, projection_matrix_, viewport);
		glm::vec3 c = glm::cross(wc_end - wc_start, look_);

		glm::mat4 rot = glm::rotate(rotation_speed_,  c);
		b->sRotation[0] = rot * b->sRotation[0];
		b->sRotation[1] = rot * b->sRotation[1];
		b->sRotation[2] = rot * b->sRotation[2];
		pose_changed_ = true;
		return ;
	}

	// FIXME: highlight bones that have been moused over
	current_bone_ = -1;
	glm::vec3 world_coordinate_near =  glm::unProject(glm::vec3(current_x_, current_y_, 0.0), view_matrix_ * model_matrix_, projection_matrix_, viewport);

	float t = 99999;
	for (int n = 1; n <= mesh_->getNumberOfBones(); ++n) {
		// turn camera and camera direction into bone's coordinates
		Bone* b = mesh_->skeleton.bones[n];
		glm::vec4 start = b->WorldPointFromBone(glm::vec4(0,0,0,1));
		glm::vec4 origin = glm::vec4(getCamera(),2) - start;
		glm::mat4 mod;
		mod[0] = b->BoneToWorldRotation()[2];
		mod[1] = b->BoneToWorldRotation()[0];
		mod[2] = b->BoneToWorldRotation()[1];
		mod[3] = b->BoneToWorldRotation()[3];
		origin = mod * origin;
		glm::vec4 dir = glm::vec4(glm::normalize(world_coordinate_near - getCamera()), 1);
		dir = mod  * dir;

		// if (n == 76) {
		// 	std::cout << "absRotation: " ;
		// 	std::cout << glm::to_string(b->BoneToWorldRotation()) << std::endl;;
		// 	std::cout << "origin: " << glm::to_string(origin) << std::endl;
		// 	std::cout << "dir   : " << glm::to_string(dir) << std::endl;
		// }
		float tt;
		if (IntersectCylinder(glm::vec3(origin), glm::vec3(dir), 0.5, b->length, &tt)) {
			if (tt < t) {
				// std::cout<<"__current bone: "<<n<<"__"<<std::endl;
				if (setCurrentBone(n)) {
					// empty

				} else {
					std::cout << "GUI BUG: attempted to set bone, but failure?" << std::endl;
				}
			}
			
		}
	}
	// std::cout << "Current bone: " << getCurrentBone() << std::endl;
	
}

void GUI::mouseButtonCallback(int button, int action, int mods)
{
	drag_state_ = (action == GLFW_PRESS);
	current_button_ = button;
}

void GUI::updateMatrices()
{
	// Compute our view, and projection matrices.
	if (fps_mode_)
		center_ = eye_ + camera_distance_ * look_;
	else
		eye_ = center_ - camera_distance_ * look_;

	view_matrix_ = glm::lookAt(eye_, center_, up_);
	light_position_ = glm::vec4(eye_, 1.0f);

	aspect_ = static_cast<float>(window_width_) / window_height_;
	projection_matrix_ =
		glm::perspective((float)(kFov * (M_PI / 180.0f)), aspect_, kNear, kFar);
	model_matrix_ = glm::mat4(1.0f);
}

MatrixPointers GUI::getMatrixPointers() const
{
	MatrixPointers ret;
	ret.projection = &projection_matrix_[0][0];
	ret.model= &model_matrix_[0][0];
	ret.view = &view_matrix_[0][0];
	return ret;
}

bool GUI::setCurrentBone(int i)
{
	if (i < 0 || i >mesh_->getNumberOfBones())
		return false;
	current_bone_ = i;
	return true;
}

bool GUI::captureWASDUPDOWN(int key, int action)
{
	if (key == GLFW_KEY_W) {
		if (fps_mode_)
			eye_ += zoom_speed_ * look_;
		else
			camera_distance_ -= zoom_speed_;
		return true;
	} else if (key == GLFW_KEY_S) {
		if (fps_mode_)
			eye_ -= zoom_speed_ * look_;
		else
			camera_distance_ += zoom_speed_;
		return true;
	} else if (key == GLFW_KEY_A) {
		if (fps_mode_)
			eye_ -= pan_speed_ * tangent_;
		else
			center_ -= pan_speed_ * tangent_;
		return true;
	} else if (key == GLFW_KEY_D) {
		if (fps_mode_)
			eye_ += pan_speed_ * tangent_;
		else
			center_ += pan_speed_ * tangent_;
		return true;
	} else if (key == GLFW_KEY_DOWN) {
		if (fps_mode_)
			eye_ -= pan_speed_ * up_;
		else
			center_ -= pan_speed_ * up_;
		return true;
	} else if (key == GLFW_KEY_UP) {
		if (fps_mode_)
			eye_ += pan_speed_ * up_;
		else
			center_ += pan_speed_ * up_;
		return true;
	}
	return false;
}


// Delegrate to the actual GUI object.
void GUI::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	GUI* gui = (GUI*)glfwGetWindowUserPointer(window);
	gui->keyCallback(key, scancode, action, mods);
}

void GUI::MousePosCallback(GLFWwindow* window, double mouse_x, double mouse_y)
{
	GUI* gui = (GUI*)glfwGetWindowUserPointer(window);
	gui->mousePosCallback(mouse_x, mouse_y);
}

void GUI::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	GUI* gui = (GUI*)glfwGetWindowUserPointer(window);
	gui->mouseButtonCallback(button, action, mods);
}
