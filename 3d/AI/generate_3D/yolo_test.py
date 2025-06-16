from ultralytics import YOLO

# YOLOモデルのロード（事前トレーニング済みのモデルを使用）
model = YOLO('yolov8n.pt')  # または他のバージョンのモデル yolov8s.pt など

# 画像のロードと解析
results = model("test.jpg")

# 結果を表示
for result in results:
    print(result.names)  # モデルが認識したクラス名
    print(result.boxes)  # バウンディングボックスの情報
#    print(result.scores)  # 各物体の信頼度スコア

#print(dir(results))
# 結果を画像として保存する場合
for result in results:
    result.save("output/")
