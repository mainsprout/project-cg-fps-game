# CG_FPS_Game
2024-2 컴퓨터 그래픽스 100/100 을 받은 프로젝트입니다.

## I. 프로젝트 개요

OpenGL을 이용하여 FPS(First-Person Shooter) 게임을 구현하였다.

라이브러리 링크 등의 문제를 피하기 위해 제공된 Tutorial의 템플릿을 사용하였으며, 코드는 하나의 cpp 파일 내에서 작성하였다.

## II . 필수 구현 요소

### 1) 총기 구현

![image.png](https://prod-files-secure.s3.us-west-2.amazonaws.com/e975e76a-cf5c-4404-816c-b88719d65c34/f5c27d08-bb4a-4e97-b14d-6200a353d49b/image.png)

총구, 몸통, 개머리판, 탄창, 손잡이로 이루어진 총기를 구현하였다. 총기를 구성하기 위해 `cube`, `torus`, `cylinder`를 사용하였다.

총기에는 총 3개의 텍스처가 사용되었다.

- `TextureBody`
- `TextureMetalBar`
- `TextureMetal`

렌더링을 하기 위해서 renderModel이라는 함수를 새로 작성하여 사용하였다.

```cpp
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
```

### 2) 배경 구현

배경은 plane 2개로 1개의 판을 구성하였다. 총 5개의 plane 판으로 천장이 뚫린 직육면체 공간을 만들었고 텍스처 또한 집어넣었다.

추가적으로 작성된 서로 다른 3가지 오브젝트들은 다음과 같다.

1. 책상
2. 과녁판
3. 드럼통

### 3) 이동, 카메라 구현

- 키보드 W, S, A, D로 각각 전, 후, 좌, 우 이동
    
    이는 `computeKeyboardTranslates` 함수를 통해 구현하였다. 
    
    ```cpp
    
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
    			bullet.modelMatrix = glm::translate(glm::mat4(1.0f), translateFactor) 
    			* bullet.modelMatrix;
    		}
    	}
    
    	lastTime = currentTime;
    }
    ```
    
    또한, `isPositionValid`를 통해 플레이어가 이동하려고 하는 공간이 유효한 공간인지 확인한다. 만약 유효한 위치가 아니라면 이동하지 않는다.
    
    ```cpp
    bool isPositionValid(const glm::vec3& position) {
    	// 벽과의 충돌 여부 확인
    	if (position.x > 40 || position.x < -10 || position.z > 30 || position.z < -30) {
    		return false;
    	}
    	return true;
    }
    ```
    

- 마우스 드래그를 이용하여 카메라를 회전
    
    이는 `computeMouseRotates` 함수를 통해 구현하였다. 또한 마우스가 움직이더라도 창 밖으로 나가지 않도록 항상 화면의 중심 위치로 고정하였다.
    
    ```cpp
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
    ```
    
    ```cpp
    void initMouseControl(GLFWwindow* window) {
    	// 마우스 커서를 비활성화하고 창 중앙에 고정
    	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    ```
    

### 4) FPS 시점 구현

무기는 플레이어를 기준으로 한 로컬 좌표계의 같은 위치에 위치하도록 구현하였다. 플레이어가 움직이거나, 카메라를 회전하여도 무기는 항상 오른쪽 하단에 위치하도록 구현하였다.

### 5) 총알 발사 구현

총알은 스페이스 바를 누르면 발사되도록 구현하였다.

먼저, `handleInput`을 통해 스페이스 입력을 감지하여 `shootBullet` 함수를 호출한다. 중복 입력을 방지하기 위해 버튼을 누르는 동안은 한 번만 호출되도록 구현하였다.

```cpp
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

```

그리고 `shootBullet`을 통해 새로운 Bullet 객체를 만든다. 여러 번의 총알 발사도 가능하다.

```cpp
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
```

`updateBullets`을 통해 총알의 위치를 업데이트 한다. 총알에는 중력 가속도도 지정하였다.

```cpp
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
```

마지막으로, `renderBullets`을 통해 활성화된 총알을 렌더링한다.

```cpp
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
```

## III. 추가 구현 요소

### 1) 모델링

총기를 매우 정성스럽게 모델링하였다. 텍스처 또한 적절하게 잘 사용하도록 노력하였다.

### 2) 총알을 여러 개 발사할 수 있도록 구현

앞서 설명했다 싶이 총알을 여러 번 발사할 수 있도록 구현하였다.

### 3) 총알이 중력가속도의 영향을 받아 점점 떨어지는 효과 구현

updateBullets에서 해당 내용을 확인할 수 있다.

```cpp
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
```
