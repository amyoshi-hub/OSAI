use pnet::transport::{transport_channel, TransportChannelType::Layer4, TransportProtocol};
use pnet::packet::udp::UdpPacket;
use pnet::packet::Packet;
use std::net::Ipv4Addr;
use std::fs::File;
use std::io::Write;

const END_SIG: u32 = 0xFFFFFFFF;

fn main() {
    let protocol = TransportProtocol::Ipv4(pnet::packet::ip::IpNextHeaderProtocols::Udp);
    let (mut _tx, mut rx) = transport_channel(4096, Layer4(protocol))
        .expect("Failed to create transport channel");

    let mut expected_chunk: u32 = 0;
    let mut file = File::create("received_image.png").expect("Failed to create file");

    println!("Waiting for UDP packets...");

    loop {
        let mut packet_iter = pnet::transport::udp_packet_iter(&mut rx);
        if let Ok((packet, _addr)) = packet_iter.next() {
            // パケットのペイロードを取得
            let payload = packet.payload();

            // ペイロード先頭4バイトをチャンク番号として読む（ビッグエンディアン）
            if payload.len() < 4 {
                eprintln!("Payload too short");
                continue;
            }
            let chunk_num = u32::from_be_bytes([payload[0], payload[1], payload[2], payload[3]]);

            if chunk_num == END_SIG {
                println!("End of transmission");
                break;
            }

            if chunk_num == expected_chunk {
                let data = &payload[4..];
                file.write_all(data).expect("Failed to write to file");
                println!("Received chunk {}", chunk_num);
                expected_chunk += 1;
            } else {
                println!("Out-of-order chunk: expected {}, got {}", expected_chunk, chunk_num);
                // 再送要求やバッファリングはここで処理可能
            }
        }
    }

    println!("File saved as received_image.png");
}

