// Markus Buchholz, 2023

#include <stdafx.hpp>
#include "imgui_helper.hpp"
#include <tuple>
#include <thread>
#include <chrono>
#include <vector>
#include <math.h>
#include <list>
#include <numeric>
#include <memory>

#include <Eigen/Dense>

//----------- system dynamic parameters --------------------

float m1 = 0.19;
float m2 = 0.1;
float k1 = 10.0;
float k2 = 5.0;
float k3 = 3.0;

float b1 = 0.0;
float b2 = 0.0;
float b3 = 0.0;

float dt = 0.001;
//-----------------------------------------------------------

std::tuple<float, float> computeEigenVectors()
{

	auto a0 = [=]
	{ return (k1 + k2) / m1; }();
	auto a1 = [=]
	{ return (-k2) / m1; }();
	auto a2 = [=]
	{ return (-k2) / m2; }();
	auto a3 = [=]
	{ return (k2 + k3) / m2; }();

	Eigen::Matrix2d matrix;

	matrix << a0, a1, a2, a3;

	Eigen::EigenSolver<Eigen::Matrix2d> solver(matrix);
	Eigen::Vector2cd eigenvalues = solver.eigenvalues();

	Eigen::MatrixXcd eigenvectors = solver.eigenvectors();

	return std::make_tuple((float)eigenvectors.coeff(0, 0).real(), (float)eigenvectors.coeff(0, 0).real());
}

//-----------------------------------------------------------
// x1_dot
float function1(float x1, float x2, float x1_dot, float x2_dot)

{

	return x1_dot;
}

//-----------------------------------------------------------
// x2_dot
float function2(float x1, float x2, float x1_dot, float x2_dot)
{
	return x2_dot;
}
//-----------------------------------------------------------
// x1_dot_dot
float function3(float x1, float x2, float x1_dot, float x2_dot)
{

	float x1_dot_dot = (-(b1 + b2) * x1_dot + b2 * x2_dot - (k1 + k2) * x1 + k2 * x2) / m1;
	return x1_dot_dot;
}

//-----------------------------------------------------------
// x2_dot_dot
float function4(float x1, float x2, float x1_dot, float x2_dot)
{
	float x2_dot_dot = (b2 * x1_dot - (b2 + b3) * x2_dot + k2 * x1 - (k2 + k3) * x2) / m2;

	return x2_dot_dot;
}

//-----------------------------------------------------------

struct Mass
{

	float x0;
	float y0;
	float x1;
	float y1;

	Mass(float x, float y)
	{

		x0 = x - 50.0f;
		y0 = y - 50.0f;
		x1 = x + 50.0f;
		y1 = y + 50.0f;
	};

	void update(float xi)
	{

		x0 = xi - 50.0f;
		y0 = 400.0f - 50.0f;
		x1 = xi + 50.0f;
		y1 = 400.0f + 50.0f;
	}
};

//-----------------------------------------------------------

int main(int argc, char const *argv[])
{
	ImVec4 clear_color = ImVec4(0.0f / 255.0, 0.0f / 255.0, 0.0f / 255.0, 1.00f);
	ImVec4 white_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	ImVec4 pink_color = ImVec4(245.0f / 255.0, 5.0f / 255.0, 150.0f / 255.0, 1.00f);
	ImVec4 blue_color = ImVec4(0.0f / 255.0, 0.0f / 255.0, 2550.0f / 255.0, 1.00f);
	int flag = true;

	int w = 800;
	int h = 500;
	std::string title = "Eigen value problem for coupled oscillators";
	initImgui(w, h, title);

	float k11{0.0f}, k12{0.0f}, k13{0.0f}, k14{0.0f};
	float k21{0.0f}, k22{0.0f}, k23{0.0f}, k24{0.0f};
	float k31{0.0f}, k32{0.0f}, k33{0.0f}, k34{0.0f};
	float k41{0.0f}, k42{0.0f}, k43{0.0f}, k44{0.0f};

	// init values
	auto eigenVect = computeEigenVectors();
	float x1 = std::get<0>(eigenVect) * (-100.0f);  // init position of mass in x
	float x2 = std::get<0>(eigenVect) * (100.0f);   // init position of masss in y
	float x3 = 0.0f;   								// init velocity
	float x4 = 0.0f;   								// init velocity
	float t = 0.0f;	   								// init time
	int ij = 0;
	std::list<ImVec2> tail;

	Mass m1(0.0f, 400.0f);
	Mass m2(0.0f, 400.0f);

	while (!glfwWindowShouldClose(window) && flag == true)
	{
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();

		ImGuiWindowFlags window_flags = 0;
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_FirstUseEver);
		window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoCollapse;
		window_flags |= ImGuiWindowFlags_NoBackground;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;

		ImGui::Begin("Simulation", nullptr, window_flags);
		ImDrawList *draw_list = ImGui::GetWindowDrawList();

		k11 = function1(x1, x2, x3, x4);
		k12 = function2(x1, x2, x3, x4);
		k13 = function3(x1, x2, x3, x4);
		k14 = function4(x1, x2, x3, x4);

		k21 = function1(x1 + dt / 2 * k11, x2 + dt / 2 * k12, x3 + dt / 2 * k13, x4 + dt / 2 * k14);
		k22 = function2(x1 + dt / 2 * k11, x2 + dt / 2 * k12, x3 + dt / 2 * k13, x4 + dt / 2 * k14);
		k23 = function3(x1 + dt / 2 * k11, x2 + dt / 2 * k12, x3 + dt / 2 * k13, x4 + dt / 2 * k14);
		k24 = function4(x1 + dt / 2 * k11, x2 + dt / 2 * k12, x3 + dt / 2 * k13, x4 + dt / 2 * k14);

		k31 = function1(x1 + dt / 2 * k21, x2 + dt / 2 * k22, x3 + dt / 2 * k23, x4 + dt / 2 * k24);
		k32 = function2(x1 + dt / 2 * k21, x2 + dt / 2 * k22, x3 + dt / 2 * k23, x4 + dt / 2 * k24);
		k33 = function3(x1 + dt / 2 * k21, x2 + dt / 2 * k22, x3 + dt / 2 * k23, x4 + dt / 2 * k24);
		k34 = function4(x1 + dt / 2 * k21, x2 + dt / 2 * k22, x3 + dt / 2 * k23, x4 + dt / 2 * k24);

		k41 = function1(x1 + dt * k31, x2 + dt * k32, x3 + dt * k33, x4 + dt * k34);
		k42 = function2(x1 + dt * k31, x2 + dt * k32, x3 + dt * k33, x4 + dt * k34);
		k43 = function3(x1 + dt * k31, x2 + dt * k32, x3 + dt * k33, x4 + dt * k34);
		k44 = function4(x1 + dt * k31, x2 + dt * k32, x3 + dt * k33, x4 + dt * k34);

		x1 = x1 + dt / 6.0 * (k11 + 2 * k21 + 2 * k31 + k41);
		x2 = x2 + dt / 6.0 * (k12 + 2 * k22 + 2 * k32 + k42);
		x3 = x3 + dt / 6.0 * (k13 + 2 * k23 + 2 * k33 + k43);
		x4 = x4 + dt / 6.0 * (k14 + 2 * k24 + 2 * k34 + k44);

		ij++;

		m1.update((x1 + 200.0) * 1.5f);
		m2.update((x2 + 400.0) * 1.5f);


		draw_list->AddLine({0.0, 400.0f}, {(float)(x1 + 200.0) * 1.5f, 400.0f}, ImColor(pink_color), 2.0f);
		draw_list->AddLine({800.0f, 400.0f}, {(float)(x2 + 400.0) * 1.5f, 400.0f}, ImColor(blue_color), 2.0f);
		draw_list->AddLine({(float)(x1 + 200.0) * 1.5f, 400.0f}, {(float)(x2 + 400.0) * 1.5f, 400.0f}, ImColor(white_color), 2.0f);

		draw_list->AddRectFilled({m1.x0, m1.y0}, {m1.x1, m1.y1}, ImColor(pink_color));
		draw_list->AddRectFilled({m2.x0, m2.y0}, {m2.x1, m2.y1}, ImColor(blue_color));


		ImGui::End();

		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	termImgui();
	return 0;
}