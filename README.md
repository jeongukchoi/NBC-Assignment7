# 7번 과제 - Pawn 클래스로 캐릭터 만들기
[시연 영상](https://youtu.be/Au0_VljpRjo)

## 구현한 내용

### 필수

- Pawn
  - BoxComponent 를 루트 컴포넌트로 설정
  - SkeletalMeshComponent, SpringArm, CameraComponent 를 Attach 하여 3인칭 시점 구성
  - Blueprint 로 감싸 메쉬 및 컴포넌트 위치 조정
  - 충돌 컴포넌트 및 메쉬 컴포넌트 -> SetSimulatePhysics(false)
  - SetupPlayerInputComponent() 에서 인풋 액션에 각각 트리거 상태에 맞는 메서드 바인딩
    - Look : 마우스 이동으로 카메라 회전
      - AddRelativeRotation 으로 카메라의 Pitch / Yaw 회전
      - 짐벌락 방지 위해 Pitch 제한
    - MoveGround : WASD 키로 지상 이동
      - SetActorRotation 으로 폰을 카메라의 Yaw 와 일치시킴
      - SpringArmComponent 의 relative 회전 값 중 pitch 만 유지하고 나머지는 0 (pawn 과 일치시킴)
      - 카메라와 폰의 yaw (즉, 바라보는 앞 방향) 가 일치하므로 액터의 Forward/Right Vector 에 입력값과 속도를 곱해 AddActorWorldOffset
      - bSweep (i.e. AddActorworldOffset 의 2번째 파라미터) true 로 설정하여 충돌 감지 ON

- IMC / IA
  - 에디터에서 사용자의 입력에 대응되는 행동 Input Action 에셋 생성
  - 에디터에서 Input Mapping Context 로 Input Action 과 플레이어의 입력값 맵핑

- PlayerController
  - IMC 및 Input Action 변수 선언
  - BeginPlay 에서 Enhanced Input Local Player Subsystem 에 IMC 등록
  - 에디터에서 블루프린트로 감싸고 IMC 및 IA 변수에 실제 에셋 지정
 
### 도전
(비행 구현에 있어 GTA 의 헬기 조종 방식 및 원리를 참고함)

- 중력 - 월드의 Z 아래 방향으로 9.8m/s^2
  - Tick 에서 2가지 경우로 나누어 적용
    1. pawn 이 공중에 있음 & 가속 중이 아님
      - AddActorWorldOffset(속력 = 낙하 시간 * 중력 가속도) --> 점점 빨라지며 낙하
    2. pawn 이 공중에 있음 & 가속 중 (i.e. 플레이어가 W키 입력 중)
      - AddActorWorldOffset(중력 가속도) --> 비행체의 가속 벡터 (Accelerate 함수 내 AddActorWorldOffset 호출) 와의 합연산으로 최종 이동 방향과 속도가 결정됨

- 6 DOF 비행
  - 별도의 IMC 로 비행에 맞는 입력 및 인풋 액션 맵핑
    - W/S : 메쉬 기준 Up 방향 가속/감속
      - 위에 설명하였듯이, 메쉬 기준 Up 벡터와 중력 벡터의 합연산으로 이동 방향 및 속력이 결정됨
      - 트리거 이벤트 Completed 에 StopAccelerate 함수 바인드 --> Tick 에서 자유낙하가 실행되도록 함 (bool 값 true 로)
    - A/D : 메쉬 Yaw 회전
    - Numpad 4/6 : 메쉬 Roll 회전
    - Numpad 8/5 : 메쉬 Pitch 회전
      - 트리거 이벤트 Completed 에 Reset 합수 바인드 --> Tick 에서 Pitch/roll 을 0으로 리셋하는 부분 실행되도록 함 (bool 값 true 로)
        - Lerp 로 Pitch / Roll 회전값이 부드럽게 0으로 돌아오도록 함

- LineTraceFloor() : 액터(pivot) 위치 ~ 박스 컴포넌트의 아랫면 (+ 어느 정도 offset) 까지 line trace 하여 공중에 떠 있는지 판단
  - 공중에 있는지 여부 업데이트 (bool)
  - Tick 에서 실행되는 자유 낙하 OFF
  - Tick 에서 실행되는 pitch/roll 리셋 OFF
  - 메쉬의 로테이션 초기화
  - IMC 교체 (지상 <-> 공중)
    - SwitchIMC(bool) : true의 경우 비행용 IMC 로, false의 경우 지상용 IMC 로 교체하도록 하는 함수



