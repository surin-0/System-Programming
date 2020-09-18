# System-Programming
2019-2학기 아주대학교 소프트웨어학과 시스템프로그래밍 과제

## gpio
(팀과제) **2개의 라즈베리파이의 통신**

- 두 개의 라즈베리 파이의 GPIO 14번, 15번 핀을 이용한 유선통신
- 디바이스 드라이버의 `open`, `release`, `write`, `read` 함수 구현
- 자세한 내용은 [보고서](https://github.com/surin-0/System-Programming/blob/master/gpio/report.pdf)참고

## sicTool
(개인과제) sic 및 sic/xe를 이용한 알고리즘 작성

- 입력 받은 정수에 대한 소인수 분해 진행
- 자세한 내용은 [보고서](https://github.com/surin-0/System-Programming/blob/master/sicTool/201720723.pdf)참고

## team_project
### 개요
- 재난 상황 시 유독 가스 혹은 유해 물질의 호흡기로의 유입을 막기 위한 스마트 방독면 제작
- 주요 기능
  1. 장애물을 감지하면 진동을 통해 알려주는 기능
  2. 방독면 내의 유독가스의 농도가 일정 수준을 초과하면 소리를 통해 알려주는 기능
  3. 방독면 내의 불쾌지수가 일정 수준을 초과하면 모터를 작동시켜 공기를 환기시켜주는 기능

### 설계
- 총 6개의 device driver와 3개의 app 구현
- 구현에 대한 자세한 내용은 [최종 보고서](https://github.com/surin-0/System-Programming/blob/master/team_project/report.pdf)를 참고
  * 시연에 사용한 센서에 대한 설명
  * 소프트웨어 구조
  * PWM 구현 방법
  * SPI 통신 구현 방법
  
