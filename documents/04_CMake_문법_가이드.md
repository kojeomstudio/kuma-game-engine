# CMake 문법 가이드

> **목적**: CMake의 기본 개념·문법 규칙·Modern CMake 작성 방식을 이해한다.
> 현재 프로젝트의 `CMakeLists.txt`를 실제 예시로 사용.
>
> **작성일**: 2026-07-17

---

## 1. CMake란? — 빌드 시스템의 "생성기"

가장 먼저 헷갈리는 개념부터 정리.

| 오해 | 실제 |
|------|------|
| CMake는 컴파일러다 | ✗ CMake는 **빌드 파일을 생성하는 생성기(generator)** |
| CMake가 직접 빌드한다 | ✗ Ninja/MSBuild/Make 같은 **실제 빌드 도구**가 컴파일 |

CMake는 "어떤 소스를, 어떤 옵션으로, 어떤 라이브러리와 링크해서 빌드할지"를 `CMakeLists.txt`로 기술하면, 그걸 읽고 **Ninja/Makefile/VS 솔루션 같은 실제 빌드 파일**을 만들어냅니다.

### 1.1 빌드의 3단계

```
CMakeLists.txt ──(configure 단계)──→ 캐시 + 프로젝트 모델
                ──(generate 단계)──→ build.ninja / .vcxproj / Makefile
                                   ──(build 단계)──→ .exe / .lib
```

| 단계 | 명령 | 역할 |
|------|------|------|
| **Configure** | `cmake --preset x64-debug` | `CMakeLists.txt` 읽고 구성 검사 (컴파일러 찾기, 옵션 평가) |
| **Generate** | (위와 같이 실행됨) | 실제 빌드 파일(`build.ninja`) 생성 |
| **Build** | `cmake --build out/build/x64-debug` | Ninja/MSBuild가 컴파일+링크 실행 |

VS IDE에서 "빌드" 버튼을 누르면 세 단계가 자동으로 처리됩니다.

---

## 2. 기본 문법 규칙

### 2.1 명령(Command) 형식

CMake의 모든 것은 **명령**입니다. 형태:

```cmake
명령이름(인자1 인자2 인자3 ...)
```

- 명령 이름은 **대소문자 구분 없음** (`project` = `PROJECT` = `Project`). 관례상 소문자 권장.
- 인자는 **공백으로 구분**, 괄호 `()` 안에.
- 인자가 문자열처럼 보여도 **따옴표가 필수는 아님**:

```cmake
project(kuma-game-engine)              # OK
project("kuma-game-engine")            # OK (동일)
project(kuma game engine)              # 3개 인자 (다른 의미!)
```

공백이나 특수문자가 있으면 따옴표로 감싸는 게 안전합니다.

### 2.2 주석

```cmake
# 한 줄 주석
```

여러 줄 주석은 없습니다. 매 줄 `#`을 붙여야 합니다. (우리 `CMakeLists.txt`의 1~3줄 참고)

### 2.3 변수

```cmake
set(MY_VAR "Hello")          # 변수에 값 할당
message(${MY_VAR})           # ${변수명}으로 참조
```

- `${...}` 로 참조 — 이게 **포인터/참조가 아니라 텍스트 치환**임을 이해하는 게 중요.
- 변수는 기본적으로 **문자열**(리스트도 사실 공백 구분 문자열).

```cmake
set(SOURCES a.cpp b.cpp c.cpp)   # 사실상 "a.cpp;b.cpp;c.cpp" 인 리스트
```

### 2.4 캐시 변수 vs 일반 변수 ⚠️ 핵심

이게 CMake에서 가장 헷갈리는 부분입니다.

```cmake
set(MY_VAR "x")              # 일반 변수 — configure 재실행 시 사라짐
set(MY_VAR "x" CACHE STRING "설명")  # 캐시 변수 — CMakeCache.txt에 저장, 영구
```

| | 일반 변수 | 캐시 변수 (`CACHE`) |
|---|---|---|
| 저장 | 메모리 | `CMakeCache.txt` 파일 |
| 재구성 시 | 매번 재평가 | 유지 (`-D`나 UI로 사용자가 바꾼 값 보존) |
| 용도 | 내부 계산 | 사용자 설정(옵션, 경로) |

캐시 변수의 전형적 패턴 (우리 `03_문서`의 GLFW 옵션에서 봄):
```cmake
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
#                                              ↑ FORCE: 이미 캐시에 있어도 덮어쓰기
```

`option()` 명령은 캐시 변수의 단축 문법:
```cmake
option(USE_OPENGL "OpenGL 백엔드 사용" ON)   # BOOL 캐시 변수 생성
```

---

## 3. CMakeLists.txt의 필수 구성 흐름

모든 `CMakeLists.txt`는 대략 이 순서를 따릅니다:

```
1. cmake_minimum_required(VERSION x.y)   ← 문법 호환성 선언 (반드시 첫 줄)
2. project(이름 ...)                      ← 프로젝트 메타데이터
3. 전역 설정 (C++ 표준 등)
4. 서드파티 의존성 (find_package / FetchContent)
5. 타겟 정의 (add_executable / add_library)
6. 타겟 속성 (target_include_directories / target_link_libraries)
```

이제 우리 프로젝트의 실제 `CMakeLists.txt`를 한 줄씩 해석하며 확인합니다.

---

## 4. 현재 CMakeLists.txt 줄별 해석

```cmake
cmake_minimum_required(VERSION 3.20)
```
- **문서 첫 줄 필수**. CMake에게 "이 문서는 3.20 문법으로 작성됨"을 알림.
- `VERSION`은 키워드 인자 (위치가 정해진 특별한 단어).
- **왜 중요한가**: 3.20 미만에서 실행 시 즉시 에러. 구문법의 뜻밖 동작 방지.

```cmake
if(POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT ...)
endif()
```
- **정책(Policy)** — 새 CMake 버전에서 동작이 바뀔 때,新旧 중 어느 동작을 쓸지 선택.
- `CMP0141`은 MSVC 디버그 정보 포맷 제어. VS Hot Reload 관련.
- 일단 "VS가 만들어준 그대로" 두면 됨. 나중에 필요 시 학습.

```cmake
project(kuma-game-engine VERSION 0.1.0 LANGUAGES CXX)
```
- 프로젝트 이름/버전 선언.
- `LANGUAGES CXX` — C++ 사용 (C는 안 쓴다고 명시). 기본값이 C+CXX라 명시 안 해도 되지만, 명시가 명확.
- 이 명령이 여러 내부 변수를 세팅: `PROJECT_NAME`, `CMAKE_CXX_COMPILER` 결정 등.

```cmake
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
```
- **C++ 표준 설정 삼총사**:
  - `CXX_STANDARD 20` → C++20 사용
  - `STANDARD_REQUIRED ON` → 컴파일러가 C++20 미지원 시 **에러** (OFF면 경고만 하고 하위로)
  - `EXTENSIONS OFF` → `gnu++20`(GCC 확장) 대신 순정 `c++20` 사용 → 이식성 ↑

```cmake
add_executable(kuma-game-engine
  sources/kuma-game-engine.cpp
  sources/kuma-game-engine.h
)
```
- **타겟(target)** 정의 — CMake의 핵심 단위.
- "kuma-game-engine이라는 실행 파일을, 이 소스들로 만들어라."
- 헤더(`.h`)는 컴파일 대상은 아니지만, VS 탐색기에 표시하려고 포함하는 게 관례.

```cmake
target_include_directories(kuma-game-engine PRIVATE sources)
```
- 이 타겟에 **헤더 탐색 경로** 추가 (`-I sources`).
- `PRIVATE` — 이 타겟 내부에서만 쓴다는 뜻 (자세한 건 5절).

---

## 5. Modern CMake의 핵심 — 타겟 기반 모델 ⭐

CMake에는 **구식(전역)**과 **Modern(타겟 기반)** 두 스타일이 있습니다. 무조건 Modern으로 쓰세요.

### 5.1 구식 (쓰지 말 것)
```cmake
# ❌ 전역 — 이 시점 이후 모든 타겟에 영향
include_directories(src)
add_definitions(-DUSE_OPENGL)
link_libraries(glfw)
```

### 5.2 Modern (이걸 쓸 것)
```cmake
# ✅ 특정 타겟에만 속성 부여
target_include_directories(my_app PRIVATE src)
target_compile_definitions(my_app PRIVATE USE_OPENGL)
target_link_libraries(my_app PRIVATE glfw)
```

### 5.3 가장 중요한 개념: PUBLIC / PRIVATE / INTERFACE

`target_*` 명령의 키워드. **의존성 전파**를 다룹니다.

| 키워드 | 의미 | 언제 쓰나 |
|--------|------|----------|
| **PRIVATE** | 이 타겟 **내부 구현**에서만 사용 | `.cpp` 안에서만 쓰는 헤더/라이브러리 |
| **INTERFACE** | 이 타겟을 **사용하는 쪽**에만 노출 | 헤더만 라이브러리 (header-only) |
| **PUBLIC** | PRIVATE + INTERFACE 둘 다 | 헤더에 노출되는 API용 |

예시로 이해:

```cmake
# glad 라이브러리 — 헤더를 PUBLIC 으로 (쓰는 쪽에서도 <glad/gl.h> 인클루드해야 함)
add_library(glad STATIC third_party/glad/src/gl.c)
target_include_directories(glad PUBLIC third_party/glad/include)
#                                      ↑ PUBLIC: glad를 링크하는 타겟도 이 경로를 받음

# 엔진 타겟
target_link_libraries(kuma-game-engine PRIVATE glfw glad)
#                                            ↑ PRIVATE: 엔진 내부에서만 GLFW/glad 사용
```

**규칙 of thumb**: 헤더 파일(`.h`)에 `#include <glfw/glfw3.h>`가 있으면 그 의존성은 **PUBLIC**, `.cpp`에만 있으면 **PRIVATE**.

---

## 6. 핵심 명령 사전

### 6.1 타겟 생성

```cmake
add_executable(이름 소스1 소스2 ...)        # 실행 파일 (.exe)
add_library(이름 STATIC 소스...)           # 정적 라이브러리 (.lib)
add_library(이름 SHARED 소스...)           # 동적 라이브러리 (.dll)
add_library(이름 INTERFACE)                # 헤더만 (인터페이스 라이브러리)
```

### 6.2 타겟 속성 (Modern CMake 핵심)

```cmake
target_include_directories(타겟 [PUBLIC|PRIVATE|INTERFACE] 경로...)
target_link_libraries(타겟 [PUBLIC|PRIVATE|INTERFACE] 라이브러리...)
target_compile_definitions(타겟 ... 정의...)        # -D 매크로
target_compile_options(타겟 ... 플래그...)          # -Wall 같은 컴파일러 플래그
target_compile_features(타겟 ... cxx_std_17)        # C++ 기능 요구
target_sources(타겟 PRIVATE 소스...)                # 소스 추가 (폴더별로 나눌 때)
```

### 6.3 의존성 해결

```cmake
# 방법 1: 시스템에 설치된 패키지 찾기
find_package(glfw3 3.4 REQUIRED)

# 방법 2: 소스를 받아 같이 빌드 (FetchContent — 우리가 쓸 방식)
include(FetchContent)
FetchContent_Declare(glfw GIT_REPOSITORY ... GIT_TAG 3.4)
FetchContent_MakeAvailable(glfw)
```

### 6.4 제어문

```cmake
if(조건)
  ...
elseif(조건2)
  ...
else()
  ...
endif()

foreach(item IN LISTS MY_LIST)
  message(${item})
endforeach()
```

조건식 예시:
```cmake
if(WIN32)                              # 운영체제가 Windows
if(CMAKE_BUILD_TYPE STREQUAL "Debug")  # 문자열 비교
if(TARGET glfw)                        # 타겟이 존재하는가
if(USE_OPENGL)                         # 변수가 참인가
```

---

## 7. 변수 스코프 (범위)

CMake 변수는 **세 가지 스코프**를 가집니다.

| 스코프 | 언제 | 특징 |
|--------|------|------|
| **전역** | 최상위 `CMakeLists.txt` | 어디서든 접근 |
| **디렉토리** | `add_subdirectory()` 진입 시 | 상위 복사본을 받아 시작, 하위로는 전파 X |
| **함수** | `function()` 진입 시 | 상위 복사본 받음. 부모로 값 돌려주려면 `PARENT_SCOPE` |

```cmake
function(my_func)
  set(X "내부값")             # 함수 안에서만
  set(X "내부값" PARENT_SCOPE) # 호출자에게 전파
endfunction()
```

서브디렉토리 패턴 (큰 프로젝트에서):
```
project/
├── CMakeLists.txt          ← add_subdirectory(engine)
└── engine/
    └── CMakeLists.txt      ← 별도 스코프
```

> 💡 **초기엔 한 파일로 충분**. 타겟이 여러 개(엔진 lib + exe + 테스트)로 늘어나면 그때 서브디렉토리로 분리.

---

## 8. 우리 프로젝트가 발전할 때의 패턴

### 8.1 지금 (단일 exe)
```cmake
add_executable(kuma-game-engine sources/main.cpp ...)
```

### 8.2 다음 단계 (엔진 lib + 실행 파일)
```cmake
# 엔진 라이브러리 (재사용 가능)
add_library(kuma_core STATIC
    sources/Core/Engine.cpp
    sources/RHI/Device.cpp
    sources/Renderer/Renderer.cpp
)
target_include_directories(kuma_core PUBLIC sources)  # PUBLIC: API 노출
target_link_libraries(kuma_core PUBLIC glfw glad glm::glm)  # 헤더에 노출되면 PUBLIC

# 실행 파일 (엔진을 사용)
add_executable(kuma_app sources/main.cpp)
target_link_libraries(kuma_app PRIVATE kuma_core)
```

### 8.3 변수로 소스 관리 (깔끔하게)
```cmake
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    "sources/*.cpp"
)
add_library(kuma_core STATIC ${ENGINE_SOURCES})
```
- `GLOB_RECURSE` — 하위 폴더까지 `.cpp` 자동 수집.
- `CONFIGURE_DEPENDS` — 소스 추가 시 CMake가 재구성 감지 (이게 없으면 새 파일 추가 시 캐시 삭제 필요).
- ⚠️ CMake 공식은 GLOB를 **경고**함 (수동 명시가 더 안전). 편의 vs 명시성 트레이드오프.

---

## 9. 자주 하는 실수 (피해야 할 것)

| 실수 | 올바른 방법 |
|------|------------|
| `include_directories()` 전역 사용 | `target_include_directories()` 타겟별 사용 |
| `add_definitions(-DFOO)` 전역 | `target_compile_definitions()` |
| `CXX_STANDARD`만 설정, `REQUIRED` 빼먹음 | 둘 다 설정 (3종 세트) |
| 변수를 `${VAR}`로 할당 | `set(VAR 값)` — `${}`는 읽을 때만 |
| `cmake_minimum_required` 생략 | 반드시 첫 줄 |
| `add_executable`에 헤더 경로 없음 | 헤더는 경로 포함해서 명시 (IDE 표시용) |
| FetchContent `MakeAvailable` 안 함 | `Declare`만 하면 실제 받아지지 않음 |

---

## 10. 학습 순서 제안

1. **지금**: 현재 `CMakeLists.txt` 이해 (4절 해석) ✅
2. **다음**: `03_문서`의 가이드대로 GLFW/glad/glm 추가 — 직접 `target_link_libraries` 작성
3. **그다음**: 엔진 lib + exe 분리 (8.2절 패턴)
4. **심화**: `CMakePresets.json` 커스터마이징, `install()` 규칙, 패키징

---

## 참고 자료

- [CMake 공식 문서](https://cmake.org/cmake/help/latest/)
- [CMake Tutorial (공식)](https://cmake.org/cmake/help/latest/guide/tutorial/index.html)
- [Modern CMake (GitBook, 인기 무료 책)](https://cliutils.gitlab.io/modern-cmake/)
- [Professional CMake: A Practical Guide (Craig Scott)](https://crascit.com/professional-cmake/)
- [FetchContent 문서](https://cmake.org/cmake/help/latest/module/FetchContent.html)
