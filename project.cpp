/* 
* 필수 채점 기준과 더불어 세 가지 요소를 추가 구현하였습니다.
* 1. 모델링
* 2. 총알 여러개 발사할 수 있도록 구현
* 3. 총알이 중력 가속도의 영향을 받아 점점 떨어지는 효과 구현
*/

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
GLFWwindow* window;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>

// 총
glm::mat4 View;
glm::mat4 Projection;
glm::mat4 Model;
glm::mat4 Model_cylinder;
glm::mat4 Model_donut;
// 총알
glm::mat4 Model_bullet;
// 벽
glm::mat4 Model_plane;

// 물체 1 : 과녁
glm::mat4 Model_target;
glm::mat4 Model_sphere;
// 물체 2 : 책상
glm::mat4 Model_desk;
// 물체 3 : 드럼통
glm::mat4 Model_drum;

// 총알 struct
struct Bullet {
	glm::mat4 modelMatrix;
	glm::vec3 direction;
	glm::vec3 velocity; 
	bool active;
	float lifetime;
};

std::vector<Bullet> bullets;
float bulletSpeed = 400.0f;
float maxBulletLifetime = 5.0f;

float horizontalAngle = 0.0f;
float verticalAngle = 0.0f;

bool firstPress = true;
double xpos_prev = 0.0;
double ypos_prev = 0.0;
double xpos = 0.0;
double ypos = 0.0;


float speed = 30.0f;
float mouseSpeed = 2.0f;

void computeMouseRotates();
void computeKeyboardTranslates();
void renderModel(const glm::mat4& ModelMatrix, GLuint programID, GLuint MatrixID, GLuint vertexbuffer, GLuint uvbuffer, GLuint Texture, const std::vector<glm::vec3>& vertices);
void handleInput();
void updateBullets(float deltaTime);
void renderBullets(GLuint MatrixID, GLuint vertexbuffer, GLuint uvbuffer, GLuint Texture, const std::vector<glm::vec3>& vertices);
void initMouseControl(GLFWwindow* window);

int main(void)
{
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(1024, 768, "12223800_dlatjgus", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	initMouseControl(window);

	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwPollEvents();
	glfwSetCursorPos(window, 1024 / 2, 768 / 2);

	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	GLuint programID = LoadShaders("TransformVertexShader.vertexshader", "TextureFragmentShader.fragmentshader");
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	
	// 기본 텍스쳐
	GLuint Texture = loadBMP_custom("uvtemplate.bmp");
	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

	// 총 텍스쳐
	GLuint TextureMetalBar = loadBMP_custom("metal_bar.bmp");
	GLuint TextureMetalBarID = glGetUniformLocation(programID, "myTextureSampler");
	GLuint TextureBody = loadBMP_custom("test2.bmp");
	GLuint TextureBodyID = glGetUniformLocation(programID, "myTextureSampler");
	GLuint TextureMetal = loadBMP_custom("metal2.bmp");
	GLuint TextureMetalID = glGetUniformLocation(programID, "myTextureSampler");

	// 과녁 텍스쳐
	GLuint TextureRed = loadBMP_custom("red.bmp");
	GLuint TextureRedID = glGetUniformLocation(programID, "myTextureSampler");
	GLuint TextureYellow = loadBMP_custom("yellow.bmp");
	GLuint TextureYellowID = glGetUniformLocation(programID, "myTextureSampler");
	GLuint TextureBlue = loadBMP_custom("blue.bmp");
	GLuint TextureBlueID = glGetUniformLocation(programID, "myTextureSampler");

	// 배경 텍스쳐
	GLuint TextureWall = loadBMP_custom("wall.bmp");
	GLuint TextureWallID = glGetUniformLocation(programID, "myTextureSampler");
	GLuint TextureWall2 = loadBMP_custom("wall2.bmp");
	GLuint TextureWallID2 = glGetUniformLocation(programID, "myTextureSampler");
	GLuint TextureGrass = loadBMP_custom("grass.bmp");
	GLuint TextureGrassID = glGetUniformLocation(programID, "myTextureSampler");

	// 책상 텍스쳐
	GLuint TextureWood = loadBMP_custom("wood.bmp");
	GLuint TextureWoodID = glGetUniformLocation(programID, "myTextureSampler");

	// 드럼통 텍스쳐
	GLuint TextureDrum = loadBMP_custom("drum.bmp");
	GLuint TextureDrumID = glGetUniformLocation(programID, "myTextureSampler");

	// 직육면체
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ("cube.obj", vertices, uvs, normals);

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

	Model_desk = glm::scale(Model_desk, glm::vec3(3.5f, 1.0f, 25.0f));
	Model_desk = glm::translate(Model_desk, glm::vec3(10.0f, -5.0f, -0.5f));

	// 원기둥
	std::vector<glm::vec3> vertices_cylinder;
	std::vector<glm::vec2> uvs_cylinder;
	std::vector<glm::vec3> normals_cylinder;
	bool res_cylinder = loadOBJ("cylinder.obj", vertices_cylinder, uvs_cylinder, normals_cylinder);

	GLuint vertexbuffer_cylinder;
	glGenBuffers(1, &vertexbuffer_cylinder);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_cylinder);
	glBufferData(GL_ARRAY_BUFFER, vertices_cylinder.size() * sizeof(glm::vec3), &vertices_cylinder[0], GL_STATIC_DRAW);

	GLuint uvbuffer_cylinder;
	glGenBuffers(1, &uvbuffer_cylinder);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer_cylinder);
	glBufferData(GL_ARRAY_BUFFER, uvs_cylinder.size() * sizeof(glm::vec2), &uvs_cylinder[0], GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, uvs_cylinder.size() * sizeof(glm::vec2), &uvs_cylinder[0], GL_STATIC_DRAW);

	Model_drum = glm::translate(Model_drum, glm::vec3(8.0f, -7.0f, 33.0f));
	Model_drum = glm::scale(Model_drum, glm::vec3(0.4f, 0.4f, 0.4f));

	// 구
	std::vector<glm::vec3> vertices_sphere;
	std::vector<glm::vec2> uvs_sphere;
	std::vector<glm::vec3> normals_sphere;
	bool res_sphere = loadOBJ("sphere.obj", vertices_sphere, uvs_sphere, normals_sphere);

	GLuint vertexbuffer_sphere;
	glGenBuffers(1, &vertexbuffer_sphere);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_sphere);
	glBufferData(GL_ARRAY_BUFFER, vertices_sphere.size() * sizeof(glm::vec3), &vertices_sphere[0], GL_STATIC_DRAW);

	GLuint uvbuffer_sphere;
	glGenBuffers(1, &uvbuffer_sphere);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer_sphere);
	glBufferData(GL_ARRAY_BUFFER, uvs_sphere.size() * sizeof(glm::vec2), &uvs_sphere[0], GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, uvs_sphere.size() * sizeof(glm::vec2), &uvs_sphere[0], GL_STATIC_DRAW);

	Model_bullet = glm::translate(Model_bullet, glm::vec3(0.0f, 5.0f, 0.0f));
	Model_bullet = glm::scale(Model_bullet, glm::vec3(0.2f, 0.2f, 0.2f));
	Model_bullet = glm::rotate(Model_bullet, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	Model_bullet = glm::rotate(Model_bullet, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	// 도넛
	std::vector<glm::vec3> vertices_donut;
	std::vector<glm::vec2> uvs_donut;
	std::vector<glm::vec3> normals_donut;
	bool res_donut = loadOBJ("torus.obj", vertices_donut, uvs_donut, normals_donut);

	GLuint vertexbuffer_donut;
	glGenBuffers(1, &vertexbuffer_donut);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_donut);
	glBufferData(GL_ARRAY_BUFFER, vertices_donut.size() * sizeof(glm::vec3), &vertices_donut[0], GL_STATIC_DRAW);

	GLuint uvbuffer_donut;
	glGenBuffers(1, &uvbuffer_donut);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer_donut);
	glBufferData(GL_ARRAY_BUFFER, uvs_donut.size() * sizeof(glm::vec2), &uvs_donut[0], GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, uvs_donut.size() * sizeof(glm::vec2), &uvs_donut[0], GL_STATIC_DRAW);

	Model_target = glm::scale(Model_target, glm::vec3(0.2f, 0.6f, 0.6f));
	Model_target = glm::rotate(Model_target, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	Model_target = glm::rotate(Model_target, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	Model_target = glm::translate(Model_target, glm::vec3(0.0f, 3.0f, 0.0f));


	// 삼각형
	std::vector<glm::vec3> vertices_plane;
	std::vector<glm::vec2> uvs_plane;
	std::vector<glm::vec3> normals_plane;
	bool res_cone = loadOBJ("plane.obj", vertices_plane, uvs_plane, normals_plane);

	GLuint vertexbuffer_plane;
	glGenBuffers(1, &vertexbuffer_plane);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_plane);
	glBufferData(GL_ARRAY_BUFFER, vertices_plane.size() * sizeof(glm::vec3), &vertices_plane[0], GL_STATIC_DRAW);

	GLuint uvbuffer_plane;
	glGenBuffers(1, &uvbuffer_plane);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer_plane);
	glBufferData(GL_ARRAY_BUFFER, uvs_plane.size() * sizeof(glm::vec2), &uvs_plane[0], GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, uvs_plane.size() * sizeof(glm::vec2), &uvs_plane[0], GL_STATIC_DRAW);

	Model_plane = glm::scale(Model_plane, glm::vec3(15.0f, 15.0f, 40.0f));
	Model_plane = glm::rotate(Model_plane, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	Projection = glm::perspective(glm::radians(90.0f), 4.0f / 3.0f, 0.1f, 100.0f);

	View = glm::lookAt(
		glm::vec3(45, 0, 0),
		glm::vec3(0, 0, 0),
		glm::vec3(0, 1, 0)
	);

	glm::mat4 CameraInverse = glm::inverse(View); // 카메라의 역변환 매트릭스 계산
	glm::vec3 gunOffset = glm::vec3(0.5, -0.5, -2.0); // 총의 카메라 대비 상대 위치
	Model = glm::translate(CameraInverse, gunOffset); // 카메라의 역변환 매트릭스에 총의 상대 위치 적용

	Model = glm::translate(Model, glm::vec3(3.0f, -1.0f, -8.0f));
	Model = glm::rotate(Model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	Model = glm::rotate(Model, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	Model = glm::scale(Model, glm::vec3(1.0f, 6.0f, 1.0f));

	double lastTime = glfwGetTime();

	do {
		glDisable(GL_CULL_FACE);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(programID);

		// 카메라 회전 구현
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		{
			if (firstPress)
			{
				glfwGetCursorPos(window, &xpos_prev, &ypos_prev);
				firstPress = false;
			}

			computeMouseRotates();
		}
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
			firstPress = true;

		// 키보드 입력 처리(WASD)
		computeKeyboardTranslates();

		// 총알 구현
		double currentTime = glfwGetTime();
		float deltaTime = float(currentTime - lastTime);
		lastTime = currentTime;
		handleInput();
		updateBullets(deltaTime);
		renderBullets(MatrixID, vertexbuffer_sphere, uvbuffer_sphere, TextureMetalBar, vertices_sphere);

		/* 벽 구현 ==============================================================*/
		glm::mat4 Model_plane2 = glm::rotate(Model_plane, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 Model_plane_bottom = glm::rotate(Model_plane, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		Model_plane_bottom = glm::translate(Model_plane_bottom, glm::vec3(2.0f, -1.0f, 0.0f));
		Model_plane_bottom = glm::scale(Model_plane_bottom, glm::vec3(2.0f, 1.0f, 1.0f));
		glm::mat4 Model_plane_bottom2 = glm::rotate(Model_plane_bottom, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 Model_plane_right1 = glm::rotate(Model_plane_bottom, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		Model_plane_right1 = glm::translate(Model_plane_right1, glm::vec3(0.0f, -1.0f, -1.0f));
		Model_plane_right1 = glm::scale(Model_plane_right1, glm::vec3(1.0f, 0.5f, 1.0f));
		glm::mat4 Model_plane_right2 = glm::rotate(Model_plane_right1, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 Model_plane_left1 = glm::translate(Model_plane_right1, glm::vec3(0.0f, 4.0f, 0.0f));
		glm::mat4 Model_plane_left2 = glm::translate(Model_plane_right2, glm::vec3(0.0f, 4.0f, 0.0f));

		glm::mat4 Model_back1 = glm::translate(Model_plane, glm::vec3(0.0f, 4.0f, 0.0f));
		glm::mat4 Model_back2 = glm::translate(Model_plane2, glm::vec3(0.0f, 4.0f, 0.0f));


		/* 총 구현 ==============================================================*/
		Model_donut = Model;
		Model_donut = glm::translate(Model_donut, glm::vec3(0.0f, 1.0f, -0.4f));
		Model_donut = glm::scale(Model_donut, glm::vec3(0.05f, 0.02f, 0.05f));

		glm::mat4 output = glm::translate(Model_donut, glm::vec3(0.0f, 98.0f, -3.0f));
		output = glm::scale(output, glm::vec3(0.4f, 0.4f, 0.4f));

		Model_cylinder = Model;
		Model_cylinder = glm::translate(Model_cylinder, glm::vec3(0.0f, 1.7f, -0.5f));
		Model_cylinder = glm::scale(Model_cylinder, glm::vec3(0.05f, 0.04f, 0.05f));

		glm::mat4 Model_cylinder2 = glm::translate(Model_cylinder, glm::vec3(0.0f, 0.0f, 22.0f));
		Model_cylinder2 = glm::scale(Model_cylinder2, glm::vec3(0.3f, 1.0f, 0.3f));
		glm::mat4 Model_cylinder3 = glm::translate(Model_cylinder, glm::vec3(0.0f, 25.0f, 0.0f));
		Model_cylinder3 = glm::scale(Model_cylinder3, glm::vec3(1.0f, 0.2f, 1.0f));
		glm::mat4 Model_cylinder4 = glm::translate(Model_cylinder2, glm::vec3(0.0f, 25.0f, 0.0f));
		Model_cylinder4 = glm::scale(Model_cylinder4, glm::vec3(1.0f, 0.2f, 1.0f));

		glm::mat4 Model2 = glm::translate(Model, glm::vec3(0.0f, 0.2f, -1.2f));
		Model2 = glm::scale(Model2, glm::vec3(1.0f, 0.8f, 0.3f));
		glm::mat4 Model3 = glm::translate(Model, glm::vec3(1.0f, 0.0f, -0.3f));
		Model3 = glm::scale(Model3, glm::vec3(0.1f, 1.0f, 0.7f));

		glm::mat4 Magazine1 = glm::translate(Model, glm::vec3(0.0f, 0.0f, 1.0f));
		Magazine1 = glm::scale(Magazine1, glm::vec3(1.0f, 0.19f, 0.4f));
		glm::mat4 Magazine2 = glm::translate(Magazine1, glm::vec3(0.0f, 0.05f, 1.8f));
		Magazine2 = glm::rotate(Magazine2, glm::radians(-3.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 Magazine3 = glm::translate(Magazine2, glm::vec3(0.0f, 0.08f, 1.9f));
		Magazine3 = glm::rotate(Magazine3, glm::radians(-5.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 Magazine4 = glm::translate(Magazine3, glm::vec3(0.0f, 0.03f, 1.8f));
		Magazine4 = glm::rotate(Magazine4, glm::radians(-2.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 Magazine5 = glm::translate(Magazine4, glm::vec3(0.0f, 0.03f, 1.8f));
		Magazine5 = glm::rotate(Magazine5, glm::radians(-2.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		glm::mat4 front1 = glm::translate(Model_cylinder, glm::vec3(0.0f, 20.0f, 0.0f));
		front1 = glm::scale(front1, glm::vec3(1.2f, 0.2f, 1.2f));
		glm::mat4 front2 = glm::translate(Model_cylinder2, glm::vec3(0.0f, 20.0f, -30.0f));
		front2 = glm::scale(front2, glm::vec3(1.4f, 0.2f, 4.0f));
		glm::mat4 front3 = glm::translate(front1, glm::vec3(0.0f, 50.0f, 0.0f));
		front3 = glm::scale(front3, glm::vec3(1.0f, 0.5f, 1.0f));
		glm::mat4 front4 = glm::translate(front2, glm::vec3(0.0f, 50.0f, 0.0f));
		front4 = glm::scale(front4, glm::vec3(1.0f, 0.5f, 1.0f));

		glm::mat4 back1 = glm::translate(Model, glm::vec3(0.0f, -1.03f, 0.3f));
		back1 = glm::scale(back1, glm::vec3(1.0f, 0.07f, 0.7f));
		back1 = glm::rotate(back1, glm::radians(-8.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 back2 = glm::translate(back1, glm::vec3(0.0f, -2.0f, 0.0f));
		back2 = glm::scale(back2, glm::vec3(1.0f, 1.1f, 1.1f));
		glm::mat4 back3 = glm::translate(back2, glm::vec3(0.0f, -2.0f, 0.0f));
		back3 = glm::scale(back3, glm::vec3(1.0f, 1.1f, 1.1f));
		glm::mat4 back4 = glm::translate(back3, glm::vec3(0.0f, -2.0f, 0.0f));
		back4 = glm::scale(back4, glm::vec3(1.0f, 1.1f, 1.1f));
		glm::mat4 back5 = glm::translate(back4, glm::vec3(0.0f, -2.0f, 0.0f));
		back5 = glm::scale(back5, glm::vec3(1.0f, 1.1f, 1.1f));
		glm::mat4 back6 = glm::translate(back5, glm::vec3(0.0f, -2.0f, 0.0f));
		back6 = glm::scale(back6, glm::vec3(1.0f, 1.1f, 1.1f));
		glm::mat4 back7 = glm::translate(back6, glm::vec3(0.0f, -2.0f, 0.0f));
		back7 = glm::scale(back7, glm::vec3(1.0f, 1.1f, 1.1f));

		glm::mat4 hand1 = glm::translate(Magazine1, glm::vec3(0.0f, -3.5f, 2.0f));
		hand1 = glm::scale(hand1, glm::vec3(1.0f, 0.5f, 4.0f));

		/* 과녁 구현 ==============================================================*/
		glm::mat4 Model_target2 = glm::scale(Model_target, glm::vec3(0.7f, 1.0f, 0.7f));
		Model_target2 = glm::translate(Model_target2, glm::vec3(0.0f, -2.0f, 0.0f));
		Model_sphere = glm::scale(Model_target, glm::vec3(0.5f, 0.3f, 0.5f));
		Model_sphere = glm::translate(Model_sphere, glm::vec3(0.0f, 0.0f, 0.0f));

		/* 책상 구현 ==============================================================*/
		glm::mat4 Model_desk2 = glm::scale(Model_desk, glm::vec3(1.0f, 8.0f, 0.1f));
		Model_desk2 = translate(Model_desk2, glm::vec3(0.0f, -1.0f, 9.0f));

		renderModel(Model_drum, programID, MatrixID, vertexbuffer_cylinder, uvbuffer_cylinder, TextureDrum, vertices_cylinder);

		renderModel(Model_desk, programID, MatrixID, vertexbuffer, uvbuffer, TextureWood, vertices);
		renderModel(Model_desk2, programID, MatrixID, vertexbuffer, uvbuffer, TextureWood, vertices);

		renderModel(Model_target, programID, MatrixID, vertexbuffer_donut, uvbuffer_donut, TextureBlue, vertices_donut);
		renderModel(Model_target2, programID, MatrixID, vertexbuffer_donut, uvbuffer_donut, TextureRed, vertices_donut);
		renderModel(Model_sphere, programID, MatrixID, vertexbuffer_sphere, uvbuffer_sphere, TextureYellow, vertices_sphere);

		renderModel(Model_plane, programID, MatrixID, vertexbuffer_plane, uvbuffer_plane, TextureWall2, vertices_plane);
		renderModel(Model_plane2, programID, MatrixID, vertexbuffer_plane, uvbuffer_plane, TextureWall2, vertices_plane);
		renderModel(Model_plane_bottom, programID, MatrixID, vertexbuffer_plane, uvbuffer_plane, TextureGrass, vertices_plane);
		renderModel(Model_plane_bottom2, programID, MatrixID, vertexbuffer_plane, uvbuffer_plane, TextureGrass, vertices_plane);
		renderModel(Model_plane_right1, programID, MatrixID, vertexbuffer_plane, uvbuffer_plane, TextureWall, vertices_plane);
		renderModel(Model_plane_right2, programID, MatrixID, vertexbuffer_plane, uvbuffer_plane, TextureWall, vertices_plane);
		renderModel(Model_plane_left1, programID, MatrixID, vertexbuffer_plane, uvbuffer_plane, TextureWall, vertices_plane);
		renderModel(Model_plane_left2, programID, MatrixID, vertexbuffer_plane, uvbuffer_plane, TextureWall, vertices_plane);
		renderModel(Model_back1, programID, MatrixID, vertexbuffer_plane, uvbuffer_plane, TextureWall2, vertices_plane);
		renderModel(Model_back2, programID, MatrixID, vertexbuffer_plane, uvbuffer_plane, TextureWall2, vertices_plane);

		renderModel(Model, programID, MatrixID, vertexbuffer, uvbuffer, TextureMetal, vertices);
		renderModel(Model2, programID, MatrixID, vertexbuffer, uvbuffer, TextureBody, vertices);
		renderModel(Model3, programID, MatrixID, vertexbuffer, uvbuffer, TextureBody, vertices);

		renderModel(Model_cylinder, programID, MatrixID, vertexbuffer_cylinder, uvbuffer_cylinder, TextureMetalBar, vertices_cylinder);
		renderModel(Model_cylinder2, programID, MatrixID, vertexbuffer_cylinder, uvbuffer_cylinder, TextureMetalBar, vertices_cylinder);
		renderModel(Model_cylinder3, programID, MatrixID, vertexbuffer_cylinder, uvbuffer_cylinder, TextureMetalBar, vertices_cylinder);
		renderModel(Model_cylinder4, programID, MatrixID, vertexbuffer_cylinder, uvbuffer_cylinder, TextureMetalBar, vertices_cylinder);

		renderModel(Model_donut, programID, MatrixID, vertexbuffer_donut, uvbuffer_donut, TextureBody, vertices_donut);
		renderModel(output, programID, MatrixID, vertexbuffer_donut, uvbuffer_donut, TextureBody, vertices_donut);

		renderModel(Magazine1,programID, MatrixID, vertexbuffer, uvbuffer, TextureMetal, vertices);
		renderModel(Magazine2, programID, MatrixID, vertexbuffer, uvbuffer, TextureMetal, vertices);
		renderModel(Magazine3, programID, MatrixID, vertexbuffer, uvbuffer, TextureMetal, vertices);
		renderModel(Magazine4, programID, MatrixID, vertexbuffer, uvbuffer, TextureMetal, vertices);
		renderModel(Magazine5, programID, MatrixID, vertexbuffer, uvbuffer, TextureMetal, vertices);

		renderModel(front1, programID, MatrixID, vertexbuffer_cylinder, uvbuffer_cylinder, TextureBody, vertices_cylinder);
		renderModel(front2, programID, MatrixID, vertexbuffer_cylinder, uvbuffer_cylinder, TextureBody, vertices_cylinder);
		renderModel(front3, programID, MatrixID, vertexbuffer_cylinder, uvbuffer_cylinder, TextureBody, vertices_cylinder);
		renderModel(front4, programID, MatrixID, vertexbuffer_cylinder, uvbuffer_cylinder, TextureBody, vertices_cylinder);

		renderModel(back1, programID, MatrixID, vertexbuffer, uvbuffer, TextureBody, vertices);
		renderModel(back2, programID, MatrixID, vertexbuffer, uvbuffer, TextureBody, vertices);
		renderModel(back3, programID, MatrixID, vertexbuffer, uvbuffer, TextureBody, vertices);
		renderModel(back4, programID, MatrixID, vertexbuffer, uvbuffer, TextureBody, vertices);
		renderModel(back5, programID, MatrixID, vertexbuffer, uvbuffer, TextureBody, vertices);
		renderModel(back6, programID, MatrixID, vertexbuffer, uvbuffer, TextureBody, vertices);
		renderModel(back7, programID, MatrixID, vertexbuffer, uvbuffer, TextureBody, vertices);

		renderModel(hand1, programID, MatrixID, vertexbuffer, uvbuffer, TextureBody, vertices);

		glfwSwapBuffers(window);
		glfwPollEvents();

	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &Texture);
	glDeleteVertexArrays(1, &VertexArrayID);

	glfwTerminate();

	return 0;
}

void renderBullets (GLuint MatrixID, GLuint vertexbuffer, GLuint uvbuffer,
	GLuint Texture, const std::vector<glm::vec3>& vertices) {
	for (const auto& bullet : bullets) {
		if (bullet.active) {
			glm::mat4 MVP = Projection * View * bullet.modelMatrix;
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, Texture);

			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

			glDrawArrays(GL_TRIANGLES, 0, vertices.size());

			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
		}
	}
}

void initMouseControl(GLFWwindow* window) {
	// 마우스 커서를 비활성화하고 창 중앙에 고정
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void computeMouseRotates() {
	static double lastTime = glfwGetTime();
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	glfwGetCursorPos(window, &xpos, &ypos);

	if (xpos != xpos_prev) {
		horizontalAngle += mouseSpeed * float(xpos_prev - xpos) * deltaTime;
	}
	if (ypos != ypos_prev) {
		verticalAngle += mouseSpeed * float(ypos_prev - ypos) * deltaTime;
	}

	glm::vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);


	glm::vec3 right = glm::vec3(
		sin(horizontalAngle - 3.14f / 2.0f),
		0,
		cos(horizontalAngle - 3.14f / 2.0f)
	);

	glm::vec3 up = glm::cross(right, direction);

	View = glm::lookAt(
		glm::vec3(45, 0, 0),          
		glm::vec3(45, 0, 0) + direction,
		up                  
	);

	xpos_prev = xpos;
	ypos_prev = ypos;

	lastTime = currentTime;

	glm::mat4 CameraInverse = glm::inverse(View); 
	glm::vec3 gunOffset = glm::vec3(0.5, -0.5, -2.0); 
	Model = glm::translate(CameraInverse, gunOffset);

	Model = glm::translate(Model, glm::vec3(3.0f, -1.0f, -8.0f));
	Model = glm::rotate(Model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	Model = glm::rotate(Model, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	Model = glm::scale(Model, glm::vec3(1.0f, 6.0f, 1.0f));
}

void updateBullets(float deltaTime) {
	glm::vec3 gravity = glm::vec3(0.0f, -9.81f, 0.0f); // 중력 가속도
	for (auto& bullet : bullets) {
		if (bullet.active) {
			// 속도에 중력 가속도 적용
			bullet.velocity += gravity * deltaTime;
			// 총알의 위치를 속도에 따라 업데이트
			bullet.modelMatrix = glm::translate(bullet.modelMatrix, bullet.velocity * deltaTime);
			bullet.lifetime -= deltaTime;
			if (bullet.lifetime <= 0.0f) {
				bullet.active = false;
			}
		}
	}
}

void shootBullet() {
	glm::mat4 CameraInverse = glm::inverse(View);
	glm::vec3 cameraPosition = glm::vec3(CameraInverse[3]);

	glm::vec3 forward = -glm::normalize(glm::vec3(View[0][2], View[1][2], View[2][2]));
	glm::vec3 bulletStartPosition = cameraPosition + forward * 25.0f; 
	glm::vec3 initialVelocity = forward * bulletSpeed;

	Bullet newBullet;
	newBullet.modelMatrix = glm::translate(glm::mat4(1.0f), bulletStartPosition);
	newBullet.modelMatrix = glm::translate(newBullet.modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
	newBullet.modelMatrix = glm::scale(newBullet.modelMatrix, glm::vec3(0.05f, 0.05f, 0.05f));
	newBullet.velocity = initialVelocity;
	newBullet.direction = forward; 
	newBullet.active = true;       
	newBullet.lifetime = maxBulletLifetime; 

	bullets.push_back(newBullet);
}

void handleInput() {
	static bool spacePressed = false;

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spacePressed) {
		shootBullet();
		spacePressed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
		spacePressed = false;
	}
}

bool isPositionValid(const glm::vec3& position) {
	// 벽과의 충돌 여부 확인
	if (position.x > 40 || position.x < -10 || position.z > 30 || position.z < -30) {
		return false;
	}
	return true;
}

void computeKeyboardTranslates()
{
	static double lastTime = glfwGetTime();

	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	glm::vec3 right = glm::normalize(glm::vec3(View[0][2], 0.0f, View[2][2]));
	glm::vec3 forward = glm::normalize(glm::vec3(View[0][0], 0.0f, View[2][0]));
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

	glm::vec3 translateFactor = glm::vec3(0.0f);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		translateFactor += right * deltaTime * speed;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		translateFactor -= right * deltaTime * speed;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		translateFactor += forward * deltaTime * speed;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		translateFactor -= forward * deltaTime * speed;
	}

	// 충돌 감지 로직 추가
	glm::vec3 proposedNewPosition = glm::vec3(Model_plane[3]) + translateFactor;
	if (!isPositionValid(proposedNewPosition)) {
		translateFactor = glm::vec3(0.0f); // 유효하지 않은 위치면 이동하지 않음
	}

	Model_plane = glm::translate(glm::mat4(1.0f), translateFactor) * Model_plane;
	Model_target = glm::translate(glm::mat4(1.0f), translateFactor) * Model_target;
	Model_desk = glm::translate(glm::mat4(1.0f), translateFactor) * Model_desk;
	Model_drum = glm::translate(glm::mat4(1.0f), translateFactor) * Model_drum;

	// 총알 위치 업데이트
	for (auto& bullet : bullets) {
		if (bullet.active) {
			bullet.modelMatrix = glm::translate(glm::mat4(1.0f), translateFactor) * bullet.modelMatrix;
		}
	}

	lastTime = currentTime;
}

void renderModel(const glm::mat4& ModelMatrix, GLuint programID, GLuint MatrixID, GLuint vertexbuffer, GLuint uvbuffer, GLuint Texture, const std::vector<glm::vec3>& vertices) {
	glm::mat4 MVP = Projection * View * ModelMatrix;

	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Texture);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glDrawArrays(GL_TRIANGLES, 0, vertices.size());

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}