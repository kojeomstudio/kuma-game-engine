# OpenGL 라이브러리 생태계 완전 정리

> **목적**: GLUT/GLFW/glad/GLEW 등 OpenGL 관련 라이브러리들의 역할·역사·선택 기준을 정리.
> 특히 "소프트웨어 렌더러" 단계에서의 OpenGL 위치를 명확히 한다.
>
> **작성일**: 2026-07-17

---

## 1. 핵심 개념 정리: 렌더링의 두 가지 방식

가장 먼저 헷갈리는 부분부터. 이 두 가지는 **완전히 다른 작업**입니다.

### 1.1 소프트웨어 렌더링 (CPU 렌더링)
```
삼각형 정점 → [내가 작성한 C++ 코드로 변환/래스터화/셰이딩] → 픽셀 배열 → 화면
```
- **CPU가 모든 계산**. 픽셀 단위로 메모리(`FrameBuffer`)에 색 채움.
- 래스터라이저(삼각형 → 픽셀), 깊이 버퍼, 셰이더 전부 **직접 구현**.
- **학습 가치 최고**: GPU가 무슨 일을 하는지 뼈대부터 이해.
- 성능: 느림 (CPU 연산). 게임 출시용이 아니라 학습용.

### 1.2 하드웨어 렌더링 (OpenGL/GPU 렌더링)
```
삼각형 정점 → [GPU에 명령 전달] → GPU가 변환/래스터화/셰이딩 → 화면
```
- **GPU가 모든 계산**. 수천 개 코어로 병렬 처리.
- 래스터라이저는 GPU 내장. 우리는 명령(`glDrawArrays`)만 내림.
- 셰이더는 GLSL로 작성해서 GPU에 업로드.
- 성능: 매우 빠름. 실제 게임이 쓰는 방식.

### 1.3 우리 프로젝트의 두 단계
```
[1단계: 소프트웨어 렌더러]                [2단계: OpenGL 렌더러]
CPU FrameBuffer + 래스터라이저 직접 구현   →   OpenGL로 GPU 활용
이때 OpenGL은 "픽셀 배열을 화면에 띄우는 용도"     이때 OpenGL은 "전체 렌더링 파이프라인"
         로만 쓰거나 아예 안 씀                       의 핵심
```

> 💡 **핵심 포인트**: 소프트웨어 렌더러 단계에서 OpenGL은 주연이 아니라
> "내가 계산한 픽셀 버퍼를 화면에 출력하는 도구"로만 쓰임 (조연).
> OpenGL이 주연이 되는 건 2단계부터.

---

## 2. "순수 OpenGL"은 왜 혼자 쓸 수 없는가?

질문: **"순수한 OpenGL을 쓰는 게 아니라 다른 사람이 래핑한 걸 써야 하나요?"**

답: **OpenGL 자체가 "혼자 쓸 수 없게" 설계되어 있습니다.** 두 가지 결핍이 있음:

### 결핍 1: 창이 없다
OpenGL은 "창을 만드는 기능"이 **명세에 없습니다**. OpenGL은 그래픽스 명령만 정의할 뿐, 화면/창/입력은 **운영체제가 담당**합니다. 그런데 운영체제별 창 API가 다름:
- Windows: `Win32 API` (WGL)
- Linux: `X11` / `Wayland` (GLX)
- macOS: `Cocoa` (CGL)

→ 크로스플랫폼 창+컨텍스트 생성을 누군가 추상화해야 함 → **GLFW/SDL/GLUT**의 존재 이유.

### 결핍 2: 함수 구현체가 드라이버 안에만 있다
앞서 배운 것 (04_문서 참고): `gl*` 함수의 구현체는 GPU 드라이버에 있고, 런타임에 함수 포인터로 로드해야 함.
→ 누군가 "함수 로드" 코드를 작성해야 함 → **glad/GLEW**의 존재 이유.

### 시각화: 왜 4개나 필요한가
```
┌─────────────────────────────────────────────┐
│  내 코드                                     │
├─────────────────────────────────────────────┤
│  ④ 수학 라이브러리 (glm)        ← 행렬/벡터  │ (선택)
├─────────────────────────────────────────────┤
│  ③ 함수 로더 (glad/GLEW)        ← gl* 로드   │ ← 결핍 2 해결
├─────────────────────────────────────────────┤
│  ② OpenGL (GPU 명령)                        │
├─────────────────────────────────────────────┤
│  ① 창/컨텍스트 (GLFW/SDL/GLUT) ← 창, 입력   │ ← 결핍 1 해결
└─────────────────────────────────────────────┘
```

---

## 3. GLUT의 역사 — 왜 오래된 책에 나오나?

### 3.1 GLUT (OpenGL Utility Toolkit, 1994년)
- Mark Kilgard가 SGI 시절 만든 **교육용** 창 툴킷.
- **1990년대 말 이후로 업데이트 없음 (사실상 방치)**.
- OpenGL 1.x 시대 (fixed-function, `glBegin/glEnd`)에 맞춰져 있음.
- 핵심 문제: **`glutMainLoop()`가 이벤트 루프를 소유** — 함수가 호출되면 영원히 반환 안 함.
  → 게임 엔진처럼 "내가 루프를 직접 돌려야 하는" 구조와 안 맞음.

### 3.2 FreeGLUT
- GLUT의 오픈소스 클론 (API 호환).
- 원본 GLUT가 죽어서 대안으로 쓰임.
- 단, "충실한 GLUT 복제"가 목표라 **현대 OpenGL에 맞춰 재설계된 건 아님**.

### 3.3 왜 오래된 책은 GLUT를 쓰나?
- 그 책이 쓰인 시점(2000년대~2010년대 초)엔 GLUT가 표준이었음.
- **현대(2025)에는 LearnOpenGL 등 현대 튜토리얼이 GLFW + glad로 대체**.
- → GLUT로 배운 개념 자체는 유효하지만, 코드 패턴은 구식.

---

## 4. 현재 (2025) 라이브러리 생태계

### 4.1 창 + 컨텍스트 생성 계층

| 라이브러리 | 상태 | 특징 | 용도 |
|-----------|------|------|------|
| **GLFW** ⭐ | 활발 | 미니멀, 현대 GL/Vulkan 1급 지원, **내가 루프 소유** | LearnOpenGL 표준, 게임 엔진 학습 |
| **SDL3** | 활발 | GLFW + 오디오/이미지/컨트롤러 통합 | 풀 프레임워크 필요 시 |
| **freeglut** | 경미 유지 | GLUT 호환, 구식 | 레거시/교육용 |
| **GLUT (원본)** | ❌ 방치 | 1990년대 멈춤 | 사용 금지 |

### 4.2 함수 로더 계층 (드라이버에서 gl* 가져오기)

| 라이브러리 | 특징 | 권장도 |
|-----------|------|--------|
| **glad2** ⭐ | 명세 기반 코드 생성기, 필요한 것만 생성, Vulkan도 지원 | ★★★★★ (신규 프로젝트) |
| glad (v1) | glad2 이전 버전, LearnOpenGL이 사용 중 | ★★★★ |
| **GLEW** | "전부 로드" 방식, 무거움, 유지보수 모드 | ★★ |
| gl3w | 가벼움, core profile 전용 | ★★★ |

### 4.3 수학 라이브러리 (선택)

| 라이브러리 | 특징 |
|-----------|------|
| **glm** | GLSL과 동일 문법, 헤더 only, 가장 인기. 학습용 추천. |
| DirectXMath | Windows/D3D 친화적, SIMD 최적화 |
| **직접 구현** | 학습 목적이면 직접 짜는 것도 좋음 |

### 4.4 이미지/자원 (선택)

| 라이브러리 | 특징 |
|-----------|------|
| **stb_image** | 헤더 1개, PNG/JPEG 로딩. 가장 단순. |
| FreeImage, libpng | 더 강력하지만 설정 복잡 |

---

## 5. GLUT vs GLFW 핵심 차이 — 루프 소유권

이거 하나가 게임 엔진에서 결정적 차이.

### 5.1 GLUT (사용 금지)
```cpp
// GLUT가 루프를 소유 — 한 번 들어가면 안 나옴
glutDisplayFunc(MyRender);      // 콜백 등록
glutMainLoop();                 // ← 여기서 영원히 루프. 반환 안 함
// 이 아래 코드는 프로그램 종료 전엔 실행 안 됨
```

### 5.2 GLFW (권장)
```cpp
// 내가 루프를 소유
while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();           // 입력 처리 (논블로킹)
    Render();                   // 내가 원할 때 그림
    glfwSwapBuffers(window);    // 버퍼 교체
}
// 루프 빠져나오면 정리 코드 자유롭게
```

게임 엔진은 프레임마다 입력→업데이트→렌더를 **내가 통제**해야 함. GLUT처럼 프레임워크가 루프를 가져가면 엔진 아키텍처가 깨짐. **이게 게임 엔진에선 GLFW가 압도적으로 유리한 이유.**

---

## 6. 추천 스택 (2025 기준)

### 6.1 학습용 게임 엔진 (우리 프로젝트)
```
GLFW 3.4  +  glad2 (GL 3.3 core)  +  glm 1.x  +  stb_image
```
- LearnOpenGL.com과 1:1 매칭 → 따라 하기 가장 쉬움
- 소프트웨어 렌더러 단계: GLFW로 창만 띄우고, 픽셀 버퍼 출력용으로 glad 최소 사용

### 6.2 상용/실무급 프레임워크
```
SDL3  +  glad2  +  (수학/이미지는 자체 또는 glm/stb)
```
- 오디오/이미지/입력 통합 필요 시

### 6.3 고성능/크로스플랫폼 엔진
```
자체 창 계층  +  bgfx 또는 자체 RHI (여러 API 추상화)
```
- 우리의 최종 목표 (검토 문서 02)

---

## 7. 우리 프로젝트에서의 역할별 사용

| 단계 | 창/입력 | 함수 로더 | 수학 | 비고 |
|------|---------|-----------|------|------|
| **1단계: 소프트웨어 렌더러** | **GLFW** | **glad2 (최소)** | glm (또는 직접) | CPU 픽셀 버퍼 → GL 텍스처/서피스로 출력만 |
| **2단계: OpenGL 렌더러** | **GLFW** | **glad2 (전체)** | glm | 본격 GL 파이프라인 |
| **3단계: RHI 추상화** | GLFW (또는 SDL3) | 각 백엔드별 | glm | GL 백엔드 + 향후 D3D 백엔드 |

> 1단계에서도 창은 GLFW로. GLUT는 절대 쓰지 마세요 — 루프 소유권 문제가 엔진 구조를 망가뜨립니다.

---

## 8. 요약 — 자주 묻는 질문 정리

**Q: GLUT로 배워도 되나요?**
A: 개념은 유효하지만, 코드 패턴이 1990년대식이라 엔진 제작엔 방해됨. 버리고 GLFW로 시작하세요.

**Q: 왜 OpenGL만 쓰면 안 되나요?**
A: OpenGL은 (1) 창 생성 기능이 명세에 없고 (2) 함수 구현체가 드라이버 안에만 있어 런타임 로드가 필요. 이 두 결핍을 메우는 보조 라이브러리가 필수.

**Q: 소프트웨어 렌더러를 작성하는데 OpenGL이 필요한가요?**
A: "픽셀을 화면에 띄우는" 부분만 필요. OpenGL의 래스터라이저를 쓰는 게 아니라, 내가 계산한 픽셀 배열을 화면에 전달하는 용도로만. GLFW + (최소) glad 조합이면 충분.

**Q: "래핑한 라이브러리"를 써야 하는 게 맞나요?**
A: 맞습니다. 단, 이건 OpenGL의 결함이라기보다 **의도된 설계** — 그래픽스 API는 OS/GPU 종속성을 분리하기 위해 이런 구조를 가짐. GLFW/glad는 그 분리를 깔끔하게 메워주는 도구.

**Q: 소프트웨어 렌더러 쓸 때 그냥 픽셀 배열만 화면에 출력하면 되나요?**
A: 네. CPU에서 `uint32_t framebuffer[WIDTH*HEIGHT]` 같은 배열을 채우고, 이걸 OpenGL 텍스처나 glDrawPixels로 한 번에 업로드해서 띄우는 패턴이 학습용 소프트웨어 렌더러의 표준.

---

## 참고 자료

- [GLFW FAQ — freeglut/SDL과의 공식 비교](https://www.glfw.org/faq)
- [FreeGLUT vs GLFW (Stack Overflow)](https://stackoverflow.com/questions/25708688/what-is-the-difference-between-freeglut-vs-glfw)
- [GLEW, GLUT, GLFW3 deprecated? (Stack Overflow)](https://stackoverflow.com/questions/61671010/what-is-glew-glut-and-glfw3-which-ones-are-deprecated-in-opengl-3-4)
- [Is GLUT obsolete? (GameDev SE)](https://gamedev.stackexchange.com/questions/23644/is-glut-obsolete)
- [glad2 GitHub (생성기)](https://github.com/Dav1dde/glad)
- [Why would you use software rendering over hardware? (GameDev SE)](https://gamedev.stackexchange.com/questions/73495/why-would-you-use-software-rendering-over-hardware-rendering-today)
- [SoftGLRender — 소프트웨어 vs OpenGL 비교 학습용 프로젝트](https://github.com/Gforcex/OpenGraphic)
- [LearnOpenGL — Hello Window](https://learnopengl.com/Getting-started/Hello-Window)
