import numpy as np
import tflite_runtime.interpreter as tflite
import time

# 1. 모델 로드
interpreter = tflite.Interpreter(model_path='/home/el/Desktop/models/yolov8s_best_float16.tflite')
interpreter.allocate_tensors()
input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()

# 2. 더미 입력 데이터 생성 (TFLite는 HWC 포맷을 사용)
# input_details[0]['shape']를 통해 정확한 입력 형태 확인 가능
dummy_input = np.random.randn(1, 640, 640, 3).astype(np.float32)

# 3. 워밍업 (Warming up)
print("\nWarming up TFLite model...")
for _ in range(10):
    interpreter.set_tensor(input_details[0]['index'], dummy_input)
    interpreter.invoke()

# 4. 속도 측정
print("Measuring TFLite model speed...")
num_runs = 100
start_time = time.time()
for _ in range(num_runs):
    interpreter.set_tensor(input_details[0]['index'], dummy_input)
    interpreter.invoke()
end_time = time.time()

# 5. 결과 계산
tflite_latency = ((end_time - start_time) / num_runs) * 1000  # ms
tflite_fps = 1 / ((end_time - start_time) / num_runs)
print(f"TFLite Latency: {tflite_latency:.2f} ms")
print(f"TFLite FPS: {tflite_fps:.2f}")