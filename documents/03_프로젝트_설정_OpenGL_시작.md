# 프로젝트 설정 및 OpenGL 시작 가이드

> **목적**: CMake 프로젝트의 인프라 수정 내역과, OpenGL 학습 엔진을 시작하기 위한
> 폴더 구조 설계·서드파티 라이브러리 포함 방법을 정리.
>
> **범위**: 인프라/설계까지만. 실제 엔진 코드 구현은 프로젝트 오너가 직접 수행.
>
> **관련**: `01_기술_검토.md`, `02_그래픽스_추상화_및_C샵_래퍼_검토.md`
>
> **작성일**: 2026-07-17

---

## 1. 인프라 수정 내역 (반영 완료)

### 1.1 `.gitignore` 보강
VS CMake 프로젝트의 산출물 패턴을 추가:
- `out/` — VS CMake 빌드 출력 디렉토리
- `.vs/` — IDE 상태 (IntelliSense DB 등)
- `*.slnx`, `*.vcxproj.user`, `slnx.sqlite` — 솔루션/사용자 파일

### 1.2 `CMakeLists.txt` 수정
- 최소 요구 버전 `3.10` → **`3.20`** (FetchContent/최신 기능 안정 지원)
- 소스 경로 불일치 수정 — `sources/kuma-game-engine.cpp`로 명시
- C++20 강제 (`CMAKE_CXX_STANDARD_REQUIRED ON`, 확장 OFF)
- `target_include_directories`로 `sources/` 헤더 탐색 경로 추가

> ⚠️ **참고**: 커맨드라인 빌드 시 `cl.exe`를 찾지 못하는 에러가 발생할 수 있음.
> 이는 **VS Developer Command Prompt** 환경에서 실행해야 PATH에 MSVC가 잡히기 때문.
> VS IDE 내에서 빌드하면 자동 처리되므로 문제없음.

---

## 2. OpenGL에 필요한 라이브러리 — 역할 분담

OpenGL은 "설치하는 라이브러리"가 아님 (`03_기술_검토` 참고). 두 종류의 도구가 필요:

```
┌─────────────────────────────────────────────┐
│  엔진 코드 (본인 작성)                        │
├─────────────────────────────────────────────┤
│  ② GL 로더 (glad2)                           │ ← 드라이버에서 gl* 함수 포인터 로드
├─────────────────────────────────────────────┤
│  ① 창 + GL 컨텍스트 (GLFW)                   │ ← "이 창은 OpenGL 쓴다" 선언
├─────────────────────────────────────────────┤
│  GPU 드라이버 (NVIDIA/AMD/Intel)              │ ← 실제 OpenGL 구현체
└─────────────────────────────────────────────┘
```

| 라이브러리 | 역할 | 버전/비고 |
|-----------|------|----------|
| **GLFW** | 창 생성, 입력, OpenGL 컨텍스트 생성 | LearnOpenGL 표준. 미니멀. |
| **glad2** | 런타임 GL 함수 포인터 로더 | 2025 권장. 명세 기반 생성. Vulkan도 지원. |
| **glm** (선택) | 수학 라이브러리 (vec/mat) | 헤더 only. 학습 후 직접 구현 시 제거 가능. |
| **stb_image** (선택) | 이미지 로딩 | 헤더 only. 텍스처 로드용. |

> glm/stb_image는 학습 단계에서 편의용. "수학 라이브러리 직접 구현"이 목표면 빼도 됨.

---

## 3. 권장 폴더 구조 (엔진 계층 반영)

검토 문서의 RHI 구조를 미리 반영한 확장 가능한 레이아웃 제안.

```
kuma-game-engine/                      ← 현재 솔루션 루트
├── documents/                         ← 설계/검토 문서
├── kuma-game-engine/                  ← 엔진 프로젝트 루트
│   ├── CMakeLists.txt
│   ├── CMakePresets.json
│   ├── sources/                       ← 본인이 작성하는 엔진 코드
│   │   ├── Core/                      ← 진입점, 엔진 루프, 윈도우
│   │   ├── Platform/                  ← 플랫폼 추상 (입력, 시간, 파일)
│   │   ├── RHI/                       ← 그래픽스 추상화 인터페이스 (IRHIDevice 등)
│   │   │   └── OpenGL/                ← GL 백엔드 구현
│   │   ├── Renderer/                  ← 고수준 렌더러 (메시, 머티리얼, 카메라)
│   │   ├── Math/                      ← 수학 (직접 구현 시)
│   │   └── main.cpp                   ← 엔진 진입점 (kuma-game-engine.cpp에서 이름 변경 권장)
│   └── third_party/                   ← 서드파티 (다음 절 참고)
│       ├── GLFW/
│       ├── glad/
│       └── ...
└── .gitignore
```

**왜 미리 계층을 나누는가?**
- 처음부터 RHI/ 백엔드/ 폴더를 만들어두면, "소프트웨어 렌더러 → GL 백엔드 → D3D 백엔드" 확장 시
  코드 이동 없이 폴더 추가로 대응 가능 (`02_검토`의 단계적 RHI 로드맵과 정렬)
- CMake는 이 폴더들을 자동으로 솔루션 탐색기에 반영 (vcxproj와 달리 수동 필터 불필요)

---

## 4. 서드파티 라이브러리 포함 방법 (3가지 옵션)

### 옵션 A: **CMake `FetchContent`** ⭐ (권장)
소스를 빌드 시점에 Git에서 받아서 같이 빌드. 의존성 추적 자동.

```cmake
# CMakeLists.txt 상단 (project() 선언 이후)
include(FetchContent)

# GLFW — 창 + GL 컨텍스트
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
)
# GLFW 옵션: 문서/테스트 빌드 끄기 (속도)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# glad2 — GL 함수 로더 (생성된 소스 사용, 옵션 B 참고)
# glm — 수학 (선택)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.3
)
FetchContent_MakeAvailable(glm)

# 엔진 타겟에 링크
target_link_libraries(kuma-game-engine PRIVATE glfw glm::glm)
```

| 장점 | 단점 |
|------|------|
| 버전 명시 가능 (재현성) | 최초 구성 시 다운로드 시간 |
| 의존성이 코드로 문서화 | 오프라인 빌드 어려움 |
| vcpkg 등 외부 패키지매니저 불필요 | |

### 옵션 B: glad2 **생성 소스 직접 포함**
glad2는 라이브러리가 아니라 **코드 생성기**. 본인이 명세를 지정해 소스를 생성한 뒤 포함.

**생성 방법 (둘 중 하나):**
1. **웹 생성기**: <https://glad.dav1dde.de/> → `glad 2` 선택
   - API: `gl:version=3.3` (core profile 권장)
   - Profile: `core`
   - Extensions: 필요한 것만 체크 (예: `GL_EXT_texture_compression_s3tc`)
   - Generate → 다운로드 → `third_party/glad/` 에 해제
2. **CLI 생성기**: `pip install glad` → `glad --generator c --api gl=3.3 --out-path third_party/glad`

**CMake 포함:**
```cmake
# glad는 생성된 소스 1개만 컴파일하면 됨 (glad.c 또는 src/gl.c)
add_library(glad STATIC third_party/glad/src/gl.c)
target_include_directories(glad PUBLIC third_party/glad/include)

target_link_libraries(kuma-game-engine PRIVATE glad)
```

> 핵심: glad2 생성 시 **필요한 GL 버전(3.3 권장)과 확장만** 선택 — 그 이상은 코드 낭비.

### 옵션 C: vcpkg (고급, 다수 라이브러리 시)
패키지 매니저로 모든 의존성 관리. 라이브러리가 많아지면 유리하지만 초기 세팅 부담.
본 단계에서는 오버킬이므로 비추천.

---

## 5. 권장 CMakeLists.txt 구조 (최종 형태)

위 옵션들을 합친, OpenGL 시작용 완전한 `CMakeLists.txt` 예시:

```cmake
cmake_minimum_required(VERSION 3.20)
project(kuma-game-engine VERSION 0.1.0 LANGUAGES CXX)

# --- C++20 ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- 서드파티 ---
include(FetchContent)

# GLFW (창 + GL 컨텍스트)
FetchContent_Declare(glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# glm (수학, 선택)
FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.3)
FetchContent_MakeAvailable(glm)

# glad2 (생성 소스 — 옵션 B 방식으로 third_party/glad/에 미리 넣어둠)
add_library(glad STATIC third_party/glad/src/gl.c)
target_include_directories(glad PUBLIC third_party/glad/include)

# --- 엔진 타겟 ---
add_executable(kuma-game-engine
    sources/main.cpp                     # 진입점
    # sources/Core/...
    # sources/Platform/...
    # sources/RHI/...
)

target_include_directories(kuma-game-engine PRIVATE sources)
target_link_libraries(kuma-game-engine PRIVATE glfw glad glm::glm)

# Windows 콘솔창 안 띄우려면 (나중에): WINDOWED 매크로로 SubSystem 전환
```

> 💡 `sources/main.cpp` 등은 본인이 직접 작성. 현재 `kuma-game-engine.cpp`를 `main.cpp`로
> 이름 변경해 Core 폴더에 두는 것을 권장 (엔진 진입점 명확화).

---

## 6. 첫 마일스톤 — "Hello Triangle" 로드맵

이 설정이 끝나면 다음 순서로 구현 (LearnOpenGL과 병행 권장):

| 스텝 | 내용 | 학습 포인트 |
|------|------|-------------|
| **0** | 빈 창 띄우기 (GLFW만) | 이벤트 루프, 컨텍스트 생성 |
| **1** | `gladLoadGL(glfwGetProcAddress)` 호출 | 로더 초기화 순서 (컨텍스트 생성 **이후**) |
| **2** |ClearColor/Clear로 단색 화면 | 첫 GL 호출, 버퍼 비우기 |
| **3** | VAO/VBO로 삼각형 | 정점 데이터 GPU 업로드 |
| **4** | 버텍스/프래그먼트 셰이더 | GLSL, 셰이더 컴파일/링크 |
| **5** | 텍스처 매핑 | UV 좌표, 텍스처 바인딩 |
| **6** | MVP 변환 (glm) | 행렬 수학, 유니폼 전달 |
| **7** | 이 단계까지 온 코드를 `RHI/` 인터페이스로 추상화 시작 | `02_검토`의 RHI 설계 적용 |

> **중요**: 스텝 0~6은 LearnOpenGL.com 을 그대로 따라가면 됨.
> 스텝 7부터가 "엔진화"의 시작 — 여기서 직접 설계가 들어감.

---

## 7. 체크리스트 (본인 작업용)

- [ ] VS IDE에서 `x64-debug` 프리셋으로 빌드 성공 확인
- [ ] `third_party/glad/` 폴더 생성 및 glad2 소스 생성/해제 (옵션 B)
- [ ] `CMakeLists.txt`에 FetchContent(GLFW, glm) + glad 추가 (본인 작성)
- [ ] `sources/` 하위에 Core/Platform/RHI/Renderer/Math 폴더 생성
- [ ] 스텝 0: GLFW로 빈 창 띄우기 구현
- [ ] 스텝 1: glad 초기화, glClear 단색 출력
- [ ] LearnOpenGL "Hello Triangle" 챕터 완료

---

## 참고 자료

- [LearnOpenGL — Hello Window](https://learnopengl.com/Getting-started/Hello-Window)
- [LearnOpenGL — Glad 설정](https://learnopengl.com/Getting-started/Creating-a-window)
- [GLFW 문서](https://www.glfw.org/docs/latest/)
- [glad2 GitHub (생성기)](https://github.com/Dav1dde/glad)
- [glad2 웹 생성기](https://glad.dav1dde.de/)
- [CMake FetchContent 가이드](https://cmake.org/cmake/help/latest/module/FetchContent.html)
- [glm GitHub](https://github.com/g-truc/glm)
