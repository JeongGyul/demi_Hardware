import torch
from ultralytics import YOLO
import time
import numpy as np

# 1. 모델 로드
model = YOLO('/home/el/Desktop/models/yolov8s_best.pt')
device = 'cuda' if torch.cuda.is_available() else 'cpu'
model.to(device)

# 2. 더미 입력 데이터 생성 (640x640 크기)
dummy_input = torch.rand(1, 3, 640, 640).to(device)

# 3. 워밍업 (Warming up) - 초기 실행은 느리므로 측정에서 제외
print("Warming up PyTorch model...")
for _ in range(10):
    _ = model(dummy_input)

# 4. 속도 측정
print("Measuring PyTorch model speed...")
num_runs = 100
start_time = time.time()
for _ in range(num_runs):
    _ = model(dummy_input)
end_time = time.time()

# 5. 결과 계산
torch_latency = ((end_time - start_time) / num_runs) * 1000  # ms
torch_fps = 1 / ((end_time - start_time) / num_runs)
print(f"PyTorch Latency: {torch_latency:.2f} ms")
print(f"PyTorch FPS: {torch_fps:.2f}")
