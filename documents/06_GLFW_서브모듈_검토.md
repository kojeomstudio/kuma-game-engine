# 기술 검토: GLFW 포크를 서브모듈로 참조하는 방안

> **이슈**: 포크한 GLFW 저장소(`git@github.com:kojeomstudio/glfw.git`)를 현재 프로젝트의
> git 서브모듈로 참조하는 방안을 검토.
>
> **작성일**: 2026-07-17

---

## 1. 핵심 통찰: "포크했다"는 게 결정적 차이

일반적인 서드파티 의존성(원본 저장소 직접 참조)이라면 FetchContent가 보통 권장됩니다.
하지만 **직접 포크한 저장소**라면 상황이 다릅니다.

| 상황 | 소스 코드 제어권 | 로컬 수정 | 학습 가치 | 적합한 방식 |
|------|-----------------|-----------|-----------|------------|
| 원본 저장소 참조 | 없음 | 불가 | 낮음 | FetchContent |
| **포크한 저장소** ⭐ | **있음** | **가능** | **높음** | **서브모듈도 합리적** |

포크했다는 건:
1. **버전/가용성을 본인이 통제** (업스트림이 사라져도 내 포크에 있음)
2. **로컬 수정/패치를 커밋**할 수 있음
3. **소스를 디스크에서 직접 읽고 학습**할 수 있음 (학습 엔진에선 큰 자산)

이 세 가지가 서브모듈의 강점과 정확히 맞물립니다.

---

## 2. 서브모듈 vs FetchContent — 본 상황 비교

| 기준 | 서브모듈 (포크 참조) | FetchContent (포크 URL) |
|------|---------------------|------------------------|
| **소스 오프라인 가용성** | ✅ 디스크에 항상 있음 | ✗ configure 시 다운로드 |
| **GLFW 소스 읽기/학습** | ✅ IDE에서 바로 탐색 | △ 빌드 폴더 깊숙이 있음 |
| **로컬 수정 + 커밋** | ✅ 포크에 직접 푸시 가능 | ✗ 빌드 캐시 수정은 날아감 |
| **버전 고정** | ✅ 특정 commit 고정 | ✅ `GIT_TAG`로 고정 |
| **클론 편의성** | ✗ `--recursive` 필요 | ✅ 일반 clone |
| **CI 복잡도** | △ recursive 처리 필요 | ✅ CMake가 자동 |
| **저장소 크기** | △ `.gitmodules`만 추적 (실제 소스는 별도) | ✅ 깔끔 |
| **업데이트** | △ 서브모듈 명령 별도 | ✅ `GIT_TAG` 변경만 |
| **detached HEAD 혼란** | ✗ 초보자 함정 | ✅ 해당 없음 |
| **의존성 추적 투명성** | ✅ `.gitmodules`로 명시 | △ CMakeLists 안에 숨음 |

### 본 프로젝트(학습 엔진 + 포크)에서 결정적 장단점

**서브모듈의 장점 (이 상황에서 빛남):**
- 🎯 **GLFW 소스를 IDE에서 바로 읽을 수 있음** — 학습 엔진에서 "GLFW가 창을 어떻게 만드나" 코드를 직접 볼 수 있는 건 큰 학습 자산
- 🎯 **포크에 커스텀 수정을 둘 수 있음** — 디버그 빌드 로그 추가, 확장 기능 실험 등
- 🎯 **버전/가용성 독립성** — 내 포크가 있으니 업스트림 정책에 휘둘리지 않음

**서브모듈의 단점:**
- ⚠️ 클론 시 `git clone --recursive` 또는 `git submodule update --init --recursive` 필요
- ⚠️ detached HEAD 상태 혼란 (서브모듈은 기본적으로 특정 commit을 가리킴)
- ⚠️ 업데이트/커밋 워크플로가 일반 git과 다름

---

## 3. 하이브리드 접근 (권장) ⭐

두 방식의 장점을 결합한 구조를 권장합니다.

```
아이디어: git 서브모듈로 소스를 디스크에 두되,
         CMake에서는 FetchContent 대신 add_subdirectory로 직접 포함
```

### 3.1 디렉토리 구조
```
kuma-game-engine/
├── .gitmodules                    ← 서브모듈 등록
├── CMakeLists.txt
└── third_party/
    └── glfw/                      ← 서브모듈 (GLFW 소스 전체)
        ├── CMakeLists.txt         ← GLFW 자체 빌드 스크립트
        ├── include/
        └── src/
```

### 3.2 서브모듈 추가 명령
```bash
# 프로젝트 루트에서
git submodule add git@github.com:kojeomstudio/glfw.git third_party/glfw
git submodule init
git submodule update

# 특정 릴리스(예: 3.4)로 체크아웃 후 고정
cd third_party/glfw
git checkout 3.4
cd ..
git add third_party/glfw
git commit -m "vendor: GLFW 3.4 as submodule"
```

### 3.3 CMakeLists.txt — add_subdirectory 방식
```cmake
# GLFW 옵션 먼저 세팅 (add_subdirectory 이전에)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

# 서브모듈로 받은 GLFW를 그대로 빌드에 포함
add_subdirectory(third_party/glfw)

target_link_libraries(kuma-game-engine PRIVATE glfw)
```

### 3.4 하이브리드가 주는 이점
- ✅ GLFW 소스가 디스크에 → IDE에서 직접 탐색/학습 가능
- ✅ 포크에 수정을 하면 내 GitHub 저장소에 커밋 가능
- ✅ CMake 설정이 단순 (`add_subdirectory` 한 줄)
- ✅ 오프라인 빌드 가능 (configure 시 다운로드 없음)
- ✅ 의존성 추적이 `.gitmodules`로 명시적

### 3.5 클론한 사람(또는 다른 PC)을 위한 안내
```bash
git clone --recursive <내저장소>
# 또는 이미 클론한 후:
git submodule update --init --recursive
```

---

## 4. 업스트림 동기화 (포크 유지보수)

포크를 쓰면 주기적으로 원본(GLFW 공식)과 동기화해야 함.

```bash
cd third_party/glfw

# 업스트림 원격 추가 (최초 1회)
git remote add upstream https://github.com/glfw/glfw.git

# 업스트림 최신 가져와서 포크에 반영
git fetch upstream
git checkout main
git merge upstream/main
git push origin main           # 내 포크에 푸시

# 프로젝트 루트에서 서브모듈 포인터 갱신
cd ../..
git add third_party/glfw
git commit -m "vendor: GLFW sync upstream"
```

---

## 5. 서브모듈 사용 시 주의함정 (반드시 숙지)

### 함정 1: detached HEAD
서브모듈은 특정 commit을 가리키며, 기본적으로 detached HEAD 상태.
→ `cd third_party/glfw && git checkout main` 으로 브랜치로 전환 후 작업.

### 함정 2: 서브모듈 안에서 커밋해도 부모가 모름
서브모듈 안에서 커밋 → 부모 저장소에서 `git add third_party/glfw`로
**포인터 갱신 커밋**을 별도로 해야 함.

### 함정 3: 실수로 서브모듈 폴더 삭제
`rm -rf third_party/glfw` 후 복구: `git submodule update --init`.

### 함정 4: `.gitmodules`와 실제 불일치
수동으로 URL 바꾸지 말 것. `git submodule set-url` 사용.

---

## 6. 결론 및 권장

### 6.1 권장안: **하이브리드 (서브모듈 + add_subdirectory)**

본 프로젝트는 **학습 목적 + 포크 보유**라는 두 조건이 서브모듈의 강점을 극대화함:
- GLFW 소스를 IDE에서 직접 읽고 학습 가능 → 그래픽스/창 생성 원리 이해에 직결
- 포크에 커스텀 수정/실험 가능 → 학습 과정에서 패치/디버깅 추가 등
- 오프라인 빌드, 버전 고정, 의존성 명시성 모두 확보

### 6.2 FetchContent이 나을 때 (참고용)
- 협업자가 많고 clone 편의성이 중요할 때
- 의존성이 수십 개 이상으로 늘어 서브모듈 관리가 부담일 때
- GLFW 소스를 읽을 일이 없을 때 (단순 사용만)

→ 본 프로젝트엔 해당하지 않음.

### 6.3 장기적 로드맵
- GLFW처럼 **학습/수정 가치가 높은 의존성** → 서브모듈 (GLFW, 향후 자체 포크한 라이브러리)
- glad (생성 소스), stb_image (헤더 1개) → **직접 폴더 포함** (서브모듈/FetchContent 불필요)
- 단순 사용만 하는 대형 의존성 → FetchContent 고려

---

## 7. 결정 필요 사항

- [ ] **서브모듈 하이브리드 방식 채택** 확정? (권장)
- [ ] GLFW 고정 버전: **3.4** (최신 안정) 확인?
- [ ] 서브모듈 경로: `third_party/glfw` 확정?
- [ ] 동기화 원격(`upstream`) 등록 시점: 지금 vs 필요할 때?

---

## 참고 자료

- [Git Tools – Submodules (공식 문서)](https://git-scm.com/book/en/v2/Git-Tools-Submodules)
- [Dependency management with CMake and git (r/cpp)](https://www.reddit.com/r/cpp/comments/9284h5/dependency_management_with_cmake_and_git/)
- [foonathan.net — CMake dependency handling](https://www.foonathan.net/2016/07/cmake-dependency-handling/)
- [EASTL #539 — game-industry perspective on submodules](https://github.com/electronicarts/EASTL/issues/539)
- [CMake and Git Submodules: Advanced Cases (dev.to)](https://dev.to/tythos/cmake-and-git-submodules-more-advanced-cases-2ka)
