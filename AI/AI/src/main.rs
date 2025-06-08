use rand::Rng; // 乱数生成のためのクレート
use std::f64; // f64::exp を使うために必要

// 定数定義
const INPUT_SIZE: usize = 2;
const HIDDEN_SIZE: usize = 1024;
const OUTPUT_SIZE: usize = 1;
const ALPHA: f64 = 0.1;
const EPOCHS: usize = 1000;

// XOR入力とターゲット
const INPUTS: [[f64; INPUT_SIZE]; 4] = [
    [0.0, 0.0],
    [0.0, 1.0],
    [1.0, 0.0],
    [1.0, 1.0],
];
const TARGETS: [f64; 4] = [0.0, 1.0, 1.0, 0.0];

/// シグモイド活性化関数
fn sigmoid(x: f64) -> f64 {
    1.0 / (1.0 + f64::exp(-x))
}

/// -1.0から1.0の範囲で乱数を生成
fn rand_init() -> f64 {
    let mut rng = rand::thread_rng();
    rng.gen_range(-1.0..=1.0) // Bevy と同じ rand クレートを使う
}

/// 隣接ノードとエラーを共有する関数
/// `errors` はスライスとして受け取る
fn shared_error(i: usize, errors: &[f64]) -> f64 {
    let mut sum = 0.0;
    let mut count = 0;
    // `i as isize` で signed integer にキャストし、負のインデックスも扱えるようにする
    for j_signed in (i as isize - 1)..=(i as isize + 1) {
        // 範囲チェック
        if j_signed >= 0 && (j_signed as usize) < errors.len() {
            sum += errors[j_signed as usize];
            count += 1;
        }
    }
    if count == 0 {
        0.0 // エラーを共有するノードがない場合は0を返す (本来はありえないはず)
    } else {
        sum / count as f64
    }
}

fn main() {
    // 重みの初期化: input -> hidden
    // C言語の `double w1[HIDDEN_SIZE][INPUT_SIZE];` に相当
    let mut w1: Vec<Vec<f64>> = (0..HIDDEN_SIZE)
        .map(|_| (0..INPUT_SIZE).map(|_| rand_init()).collect())
        .collect();

    // 重みの初期化: hidden -> output
    // C言語の `double w2[OUTPUT_SIZE][HIDDEN_SIZE];` に相当
    // OUTPUT_SIZE が 1 なので、Vec<f64> でもよい
    let mut w2: Vec<Vec<f64>> = (0..OUTPUT_SIZE)
        .map(|_| (0..HIDDEN_SIZE).map(|_| rand_init()).collect())
        .collect();

    // 訓練
    for epoch in 0..EPOCHS {
        for n in 0..4 {
            // フォワードパス (隠れ層)
            let mut hidden = vec![0.0; HIDDEN_SIZE];
            for i in 0..HIDDEN_SIZE {
                let mut sum_hidden = 0.0;
                for j in 0..INPUT_SIZE {
                    sum_hidden += w1[i][j] * INPUTS[n][j];
                }
                hidden[i] = sigmoid(sum_hidden);
            }

            // フォワードパス (出力層)
            let mut output = 0.0;
            for j in 0..HIDDEN_SIZE {
                output += w2[0][j] * hidden[j];
            }
            output = sigmoid(output);

            // エラー計算
            let error_out = TARGETS[n] - output;

            // 隠れ層のエラー計算
            let mut hidden_errors = vec![0.0; HIDDEN_SIZE];
            for i in 0..HIDDEN_SIZE {
                hidden_errors[i] = error_out * w2[0][i];
            }

            // w2 (hidden → output) の重み更新
            for j in 0..HIDDEN_SIZE {
                w2[0][j] += ALPHA * error_out * hidden[j];
            }

            // w1 (input → hidden) の重み更新 (共有エラーを使用)
            for i in 0..HIDDEN_SIZE {
                let shared = shared_error(i, &hidden_errors); // スライスとして渡す
                for j in 0..INPUT_SIZE {
                    w1[i][j] += ALPHA * shared * INPUTS[n][j];
                }
            }
        }
    }


    println!("Result after training:");
    for n in 0..4 {
        // フォワードパス (隠れ層)
        let mut hidden = vec![0.0; HIDDEN_SIZE];
        for i in 0..HIDDEN_SIZE {
            let mut sum_hidden = 0.0;
            // ここを修正: C言語風のループをRustのイテレータに
            for j in 0..INPUT_SIZE { // `for j in 0; j < INPUT_SIZE; j += 1 {` を修正
                sum_hidden += w1[i][j] * INPUTS[n][j];
            }
            hidden[i] = sigmoid(sum_hidden);
        }

        // フォワードパス (出力層)
        let mut output = 0.0;
        // ここを修正: C言語風のループをRustのイテレータに
        for j in 0..HIDDEN_SIZE { // `for j in 0; j < HIDDEN_SIZE; j += 1 {` を修正
            output += w2[0][j] * hidden[j];
        }
        output = sigmoid(output);

        println!("in: {:.0} {:.0} -> out: {:.3}", INPUTS[n][0], INPUTS[n][1], output);
    }
    
}
