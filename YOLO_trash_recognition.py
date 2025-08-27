import sys
sys.path.append('/usr/lib/python3/dist-packages')
from picamera2 import Picamera2
from ultralytics import YOLO
import cv2
import serial
import time
import math
import requests

# 아두이노 시리얼 포트 설정
ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
time.sleep(2)  # 아두이노 초기화 시간 확보

# Django 서버 주소
url = "http://192.168.1.101:8000/api/ultrasound/saturation/"

# YOLO 모델 로드
model = YOLO("/home/el/Desktop/models/yolov8s_best.pt")

# Picamera2 설정
picam2 = Picamera2()
picam2.preview_configuration.main.size = (640, 480)
picam2.preview_configuration.main.format = "RGB888"
picam2.configure("preview")
picam2.start()

# 탐지할 최소 객체 면적 (너무 작은 박스 무시)
MIN_AREA = 4000

# 중복 분류 방지를 위한 플래그
object_present = False

while True:
    frame = picam2.capture_array()
    results = model(frame)

    boxes = results[0].boxes
    detected = False  # 현재 프레임에서 유효한 객체가 탐지되었는지 여부

    if boxes:
        # 화면 중앙으로부터 제일 가까운 객체 하나만을 선택하기 위한 계산 과정

        # 프레임의 중심 좌표 계산
        frame_h, frame_w, _ = frame.shape
        center_x = frame_w / 2
        center_y = frame_h / 2

        # 중심에서 가장 가까운 박스를 탐색
        min_distance = float('inf')
        closest_cls_id = None

        for box, cls_id in zip(boxes.xyxy, boxes.cls):
            x1, y1, x2, y2 = box
            area = (x2 - x1) * (y2 - y1)

            if area < MIN_AREA:
                continue     # 너무 작은 박스는 무시
            
            # 박스의 중심 좌표 계산
            box_center_x = (x1 + x2) / 2
            box_center_y = (y1 + y2) / 2

            # 중심점으로부터 거리 계산
            distance = math.sqrt((box_center_x - center_x)**2 + (box_center_y - center_y)**2)

            # 가장 가까운 객체만 선택
            if distance < min_distance:
                min_distance = distance
                closest_cls_id = int(cls_id)

        if closest_cls_id is not None:
            detected = True   # 유효한 객체가 탐지됨

            # 중복 동작 방지를 위해 이전 프레임에서 객체가 없는 경우에만 분류 수행
            if not object_present:
                class_name = model.names[closest_cls_id]

                # 탐지된 클래스에 따라 아두이노로 명령 전송, 아두이노에서 레일 조작 수행
                if class_name == "plastic":
                    ser.write(b'P')
                elif class_name == "can":
                    ser.write(b'C')
                elif class_name == "glass":
                    ser.write(b'G')
                
                object_present = True  # 객체가 탐지됐으므로 True로 설정 (중복 분류 방지)

    if not detected:
        object_present = False  # 객체가 없으면 다음 탐지를 위해 초기화

    # 결과 이미지에 박스를 그려서 출력
    annotated_frame = results[0].plot()
    cv2.imshow("YOLOv8 Real-Time Detection", annotated_frame)

    if cv2.waitKey(1) & 0xFF == ord('q'): # q를 누르면 프로그램 종료
        break
    
    # 초음파 센서 값 Django 서버로 전송
    if ser.in_waiting:
        try:
            raw = ser.read(ser.in_waiting).decode('utf-8')
            lines = raw.strip().split('\n')
            
            for line in lines:
                #line = ser.readline().decode('utf-8').strip()
                print("Received:", line)
                values = line.split(',')
                print(values)
                if len(values) == 3:
                    d1, d2, d3 = map(int, values)
                    
                    data = {
                        "plastic": d1,
                        "glass": d3,
                        "can": d2,
                    }
                    
                    print(data)
                    response = requests.post(url, json=data, timeout=0.5)
                    print("POST status:", response.status_code)

        except Exception as e:
            print("Error:", e)
    

cv2.destroyAllWindows()
ser.close()
