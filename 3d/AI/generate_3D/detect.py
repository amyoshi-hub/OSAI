import os
import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt
from ultralytics import YOLO

# YOLOモデルのロード
model = YOLO('yolov8n.pt')

# 画像のロードと解析
results = model("cars.jpg")

for i, result in enumerate(results):
    print(f"Result {i + 1}:")
    for box in result.boxes:  # 各検出結果をループ処理
        # クラス名、信頼度、座標を取得
        cls = int(box.cls[0])  # クラスID
        conf = box.conf[0].item()  # 信頼度
        xyxy = box.xyxy[0].tolist()  # バウンディングボックス（左上x, 左上y, 右下x, 右下y）

        # クラス名を取得
        class_name = result.names[cls]

        # 出力
        print(f"  Class: {class_name} (ID: {cls})")
        print(f"  Confidence: {conf:.2f}")
        print(f"  Bounding Box: {xyxy}")

    print("-" * 40)


for i, result in enumerate(results):  # 各結果をループ処理
    annotated_image = result.plot(show=False)

    plt.figure(figsize=(10, 10))
    plt.imshow(annotated_image)
    plt.axis('off')
    plt.title(f"Result {i}")
    plt.show()
