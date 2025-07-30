import sys
sys.path.append('/usr/lib/python3/dist-packages')
from picamera2 import Picamera2
from ultralytics import YOLO
import cv2


model = YOLO("/home/el/Desktop/models/yolov8s_best.pt") 

picam2 = Picamera2()
picam2.preview_configuration.main.size = (640, 480)  
picam2.preview_configuration.main.format = "RGB888"
picam2.configure("preview")
picam2.start()

while True:
    frame = picam2.capture_array()

    results = model(frame)  
    annotated_frame = results[0].plot() 
    cv2.imshow("YOLOv8 Real-Time Detection", annotated_frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cv2.destroyAllWindows()
