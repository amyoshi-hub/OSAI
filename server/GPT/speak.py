from flask import Flask, request, jsonify
import os
from flask_cors import CORS

app = Flask(__name__)
CORS(app)  # すべてのオリジンからのリクエストを許可

@app.route('/speak', methods=['POST'])
def speak():
    data = request.get_json()
    message = data.get('message', '')
    if message:
        # espeakを呼び出して音声出力
        os.system(f'espeak-ng -v ja "{message}" --stdout | aplay')
        return jsonify({"status": "success", "message": "音声出力完了"})
    else:
        return jsonify({"status": "error", "message": "無効なメッセージ"}), 400

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
