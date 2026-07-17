# GLFW 학습 자료 가이드

> **목적**: GLFW를 학습하기 위한 공식/커뮤니티 자료를 정리하고, 추천 학습 경로를 제시.
>
> **작성일**: 2026-07-17

---

## 1. 학습 자료 — 계층별 정리

### 1.1 공식 문서 (가장 권위적, 최우선)

| 자료 | 링크 | 용도 |
|------|------|------|
| **Getting Started (Quick Guide)** | [glfw.org/docs/latest/quick_guide.html](https://www.glfw.org/docs/latest/quick_guide.html) | **출발점**. 창+컨텍스트+입력+루프의 최소 패턴을 단계별 설명. 회전하는 삼각형까지 완성. |
| **Window Guide** | [glfw.org/docs/latest/window_guide.html](https://www.glfw.org/docs/latest/window_guide.html) | 창 생성/힌트/풀스크린 등 상세 |
| **Input Guide** | [glfw.org/docs/latest/input_guide.html](https://www.glfw.org/docs/latest/input_guide.html) | 키보드/마우스/게임패드/콜백 |
| **Context Guide** | [glfw.org/docs/latest/context_guide.html](https://www.glfw.org/latest/context_guide.html) | OpenGL 컨텍스트 관리 |
| **Building Applications** | [glfw.org/docs/latest/build_guide.html](https://www.glfw.org/docs/latest/build_guide.html) | 컴파일/링크(CMake) — 이미 우리가 적용함 |
| **GLFW Discourse 포럼** | [discourse.glfw.org](https://discourse.glfw.org/) | 공식 커뮤니티, Q&A |

> 💡 공식 문서는 Doxygen으로 생성되어 **모든 함수에 클릭 가능한 레퍼런스**가 연결되어 있음.
> 튜토리얼에서 함수 클릭하면 상세 스펙으로 바로 이동.

### 1.2 LearnOpenGL (사실상 표준 튜토리얼)

| 챕터 | 링크 | 내용 |
|------|------|------|
| **Hello Window** | [learnopengl.com/Getting-started/Hello-Window](https://learnopengl.com/Getting-started/Hello-Window) | GLFW + glad 첫 창 (우리의 다음 단계) |
| **Hello Triangle** | [learnopengl.com/Getting-started/Hello-Triangle](https://learnopengl.com/Getting-started/Hello-Triangle) | 첫 삼각형 (VAO/VBO/셰이더) |
| **Creating a Window** | [learnopengl.com/Getting-started/Creating-a-Window](https://learnopengl.com/Getting-started/Creating-a-Window) | glad 설정 포함 상세 |

**왜 LearnOpenGL이 강력한가**: GLFW만 단독으로 가르치지 않고, **GLFW + glad + glm + stb_image를 실무 조합으로** 처음부터 사용. 우리 프로젝트와 1:1 매칭. 한글 번역도 있음 ([learnopengl.com-code.tistory.com](https://learnopengl.com/) 등).

### 1.3 비디오 자료

| 자료 | 링크 | 용도 |
|------|------|------|
| **Intro to GLFW** | [youtube.com/watch?v=XH82zuPSPkI](https://www.youtube.com/watch?v=XH82zuPSPkI) | VS 설정 + GLFW 기본 (창, 컨텍스트, 입력 콜백) |
| **Getting Started with GLFW** | [youtube.com/watch?v=dA991DBSZh4](https://www.youtube.com/watch?v=dA991DBSZh4) | 기본 렌더링 창 + 입력 |

### 1.4 서브모듈로 받은 GLFW 자체 예제 (우리가 직접 접근 가능!) ⭐

```
third_party/glfw/examples/
├── triangle-opengl.c    ← 공식 quick guide의 회전 삼각형 전체 코드
├── offscreen.c
├── windows.c
└── ...
```

우리가 GLFW를 **서브모듈로 디스크에 두었기 때문에** 이 예제들을 IDE에서 바로 열어볼 수 있음. 이게 서브모듈 방식의 학습적 이점 (검토 문서 06에서 언급).

---

## 2. 공식 Quick Guide의 핵심 학습 포인트 ⭐

방금 공식 quick guide를 정독했습니다. GLFW 프로그래밍의 **필수 패턴 8단계**를 정리합니다. 이게 GLFW를 익히는 뼈대입니다.

### 2.1 헤더 포함 순서 — 우리가 반드시 지켜야 할 규칙
```cpp
#define GLAD_GL_IMPLEMENTATION     // glad 구현체 활성화 (한 번만)
#include <glad/gl.h>               // glad를 먼저

#define GLFW_INCLUDE_NONE          // GLFW가 자체 GL 헤더를 포함하지 못하게
#include <GLFW/glfw3.h>            // 그 다음 GLFW
```
**왜 중요한가**: GLFW 헤더는 기본적으로 OS의 GL 헤더를 포함하려 함(Windows는 GL 1.2만). `GLFW_INCLUDE_NONE`으로 막고 glad가 관리하게 해야 함. 우리 프로젝트에서 main.cpp에 바로 적용할 패턴.

### 2.2 GLFW 프로그램의 8단계 구조
```cpp
// 1. 에러 콜백 등록 (초기화 "이전"에도 가능)
glfwSetErrorCallback(error_callback);

// 2. GLFW 초기화
if (!glfwInit()) { /* 실패 처리 */ }

// 3. GL 버전/프로파일 힌트 (창 생성 "이전")
glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

// 4. 창 + 컨텍스트 생성
GLFWwindow* window = glfwCreateWindow(640, 480, "Kuma", NULL, NULL);

// 5. 컨텍스트를 현재 스레드로 + glad 로드 (이 순서!)
glfwMakeContextCurrent(window);
gladLoadGL(glfwGetProcAddress);    // ← 컨텍스트 생성 이후에 호출

// 6. 키 콜백 등록
glfwSetKeyCallback(window, key_callback);

// 7. 메인 루프 (내가 소유)
while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT);
    // ... 렌더링 ...
    glfwSwapBuffers(window);   // 더블 버퍼 교체
    glfwPollEvents();          // 이벤트 처리
}

// 8. 정리
glfwDestroyWindow(window);
glfwTerminate();
```

### 2.3 핵심 개념 4가지 (문서에서 강조하는 것들)

| 개념 | 설명 |
|------|------|
| **더블 버퍼링** | front buffer(화면 표시) + back buffer(렌더링 대상). `glfwSwapBuffers`로 교체. |
| **VSync (swap interval)** | `glfwSwapInterval(1)`로 화면 주사율에 동기화 → 티어링 방지, CPU/GPU 절약. 기본값 0(비활성). |
| **Poll vs Wait** | `glfwPollEvents`(즉시 반환, 게임용) vs `glfwWaitEvents`(이벤트까지 대기, 에디터/툴용). |
| **창 닫기 플래그** | 창이 바로 닫히지 않음 — `glfwWindowShouldClose` 플래그만 설정됨. 우리가 루프에서 확인. |

### 2.4 콜백 기반 입력 (이벤트 처리)
GLFW는 콜백 함수로 이벤트를 전달:
```cpp
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}
```
콜백 종류: `key_callback`, `mouse_button_callback`, `cursor_pos_callback`, `scroll_callback`, `window_size_callback` 등.

---

## 3. 추천 학습 경로

### 3.1 우리 프로젝트 기반 단계별 학습

| 스텝 | 자료 | 산출물 |
|------|------|--------|
| **1. 읽기** | 공식 Quick Guide (1.1 첫 링크) 읽기 | 8단계 패턴 이해 |
| **2. 따라 하기** | LearnOpenGL "Hello Window" | main.cpp에 GLFW 창 띄우기 |
| **3. glad 통합** | 2.1절 헤더 순서 규칙 적용 | `gladLoadGL` + `glClear` 단색 출력 |
| **4. 입력 처리** | 공식 Input Guide | ESC로 종료, 방향키로 창 색 변경 |
| **5. 참고 코드** | `third_party/glfw/examples/triangle-opengl.c` 열어보기 | 회전 삼각형 완성 코드 |
| **6. 심화** | LearnOpenGL "Hello Triangle" | VAO/VBO/셰이더로 우리만의 삼각형 |

### 3.2 학습 팁
- **공식 문서 우선**: GLFW 함수의 정확한 스펙은 항상 공식 문서에서 확인 (튜토리얼 코드에 가끔 구버전/오류 있음).
- **서브모듈 활용**: `third_party/glfw/src/window.c` 등을 열어보면 "GLFW가 내부적으로 어떻게 창을 만드나"를 직접 볼 수 있음 — 학습 엔진의 큰 자산.
- **한글 LearnOpenGL**: 영어가 부담이면 번역판을 병행. 단, 번역은 업데이트가 늦을 수 있으니 최신 API는 원문 확인.

---

## 4. 우리 프로젝트의 다음 코드 (참고용)

학습 후 main.cpp에 작성할 "Hello Window" 최소 형태 — 2.2절의 8단계를 적용한 우리 버전:

```cpp
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cstdio>
#include <cstdlib>

static void error_callback(int error, const char* description) {
    std::fprintf(stderr, "GLFW Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main(void) {
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) return EXIT_FAILURE;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Kuma", NULL, NULL);
    if (!window) { glfwTerminate(); return EXIT_FAILURE; }

    glfwSetKeyCallback(window, key_callback);
    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);     // 컨텍스트 생성 이후
    glfwSwapInterval(1);                // VSync

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
```

> 이건 참고용 — 본인이 직접 LearnOpenGL을 따라 작성하는 걸 권장 (학습 효과).

---

## 5. 자료 접근성 요약

| 자료 | 언어 | 난이도 | 권장도 |
|------|------|--------|--------|
| 공식 Quick Guide | 영어 | 초중 | ★★★★★ (출발점) |
| LearnOpenGL | 영어/한글 번역 | 초중 | ★★★★★ (실습 병행) |
| 공식 Input/Window Guide | 영어 | 중 | ★★★★ (필요 시 참조) |
| YouTube 튜토리얼 | 영어 | 초 | ★★★ (시각적 학습 선호 시) |
| 서브모듈 GLFW 예제 | C 코드 | 중 | ★★★★ (직접 열어보기) |

---

## 참고 링크 모음

- [GLFW 공식 Quick Guide](https://www.glfw.org/docs/latest/quick_guide.html) — 시작점
- [GLFW Window Guide](https://www.glfw.org/docs/latest/window_guide.html)
- [GLFW Input Guide](https://www.glfw.org/docs/latest/input_guide.html)
- [GLFW Building Guide](https://www.glfw.org/docs/latest/build_guide.html)
- [GLFW GitHub Wiki](https://github.com/glfw/glfw/wiki)
- [GLFW Discourse 포럼](https://discourse.glfw.org/)
- [LearnOpenGL — Hello Window](https://learnopengl.com/Getting-started/Hello-Window)
- [LearnOpenGL — Hello Triangle](https://learnopengl.com/Getting-started/Hello-Triangle)
- [Intro to GLFW (YouTube)](https://www.youtube.com/watch?v=XH82zuPSPkI)
