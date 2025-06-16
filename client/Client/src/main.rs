use pnet::transport::{transport_channel, TransportChannelType::Layer4, TransportProtocol};
use pnet::packet::MutablePacket;
use pnet::packet::udp::MutableUdpPacket;
use pnet::packet::ip::IpNextHeaderProtocols;
use std::net::Ipv4Addr;
use std::fs::File;
use std::io::Read;

const END_SIG: u32 = 0xFFFFFFFF;
const CHUNK_SIZE: usize = 1472 - 8 - 4;  // UDP Payload max 1024 - 4bytes chunk_id

fn build_udp_packet<'a>(
    buffer: &'a mut [u8],
    src_port: u16,
    dst_port: u16,
    chunk_id: u32,
    payload: &[u8]
) -> MutableUdpPacket<'a> {
    let mut packet = MutableUdpPacket::new(buffer).expect("Failed to create UDP packet");
    packet.set_source(src_port);
    packet.set_destination(dst_port);
    packet.set_length((8 + 4 + payload.len()) as u16);
    packet.set_checksum(0);
    let packet_payload = packet.payload_mut();
    packet_payload[..4].copy_from_slice(&chunk_id.to_be_bytes());
    packet_payload[4..4 + payload.len()].copy_from_slice(payload);
    packet
}

fn file_send(src_ip: Ipv4Addr, src_port: u16, dst_ip: Ipv4Addr, dst_port: u16, filename: &str){
    let protocol = TransportProtocol::Ipv4(IpNextHeaderProtocols::Udp);
    let (mut tx, _) = transport_channel(4096, Layer4(protocol)).expect("Failed to open channel");

    let mut file = File::open(filename).expect("Failed to open file");
    let mut buffer = [0u8; CHUNK_SIZE];
    let mut chunk_id = 0u32;

    loop {
        let read_bytes = file.read(&mut buffer).expect("Failed to read file");
        if read_bytes == 0 {
            break;
        }

        let mut packet_buffer = vec![0u8; 8 + 4 + read_bytes];
        let mut packet = build_udp_packet(&mut packet_buffer, src_port, dst_port, chunk_id, &buffer[..read_bytes]);
        tx.send_to(packet, std::net::IpAddr::V4(dst_ip)).expect("Failed to send packet");

        println!("Sent chunk {}", chunk_id);
        chunk_id += 1;
    }

    let mut end_packet_buffer = vec![0u8; 8 + 4];
    let mut end_packet = build_udp_packet(&mut end_packet_buffer, src_port, dst_port, END_SIG, &[]);
    tx.send_to(end_packet, std::net::IpAddr::V4(dst_ip)).expect("Failed to send end packet");

    println!("All chunks sent");
}

fn main() {
    let src_ip = Ipv4Addr::new(172, 20, 10, 2);
    let dst_ip = Ipv4Addr::new(172, 20, 10, 2);
    let src_port = 12345;
    let dst_port = 12345;
    let filename = "test.png";
    
    file_send(src_ip, src_port, dst_ip, dst_port, filename);

    
}

