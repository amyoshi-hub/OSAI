import cv2

# カメラを開く（0は通常デフォルトカメラ）
cap = cv2.VideoCapture(0)

if not cap.isOpened():
    print("カメラを開けませんでした")
    exit()

# 1フレーム読み込み
ret, frame = cap.read()

if ret:
    # 画像を保存（.jpg）
    cv2.imwrite("captured_image.jpg", frame)
    print("画像を保存しました: captured_image.jpg")
else:
    print("フレームの取得に失敗しました")

# カメラを解放
cap.release()
