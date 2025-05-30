use pnet::packet::ip::IpNextHeaderProtocols;
use pnet::packet::udp::{MutableUdpPacket};
use pnet::packet::ipv4::MutableIpv4Packet;
use pnet::packet::MutablePacket;
use pnet::transport::{transport_channel, TransportChannelType};
use std::net::{Ipv4Addr};
use std::fs::File;
use std::io::{Read, BufReader};

const END_SIG: u32 = 0xFFFFFFFF;

/*
 *IP--20byte
-payload
    -UDP-8byte
    -id:先頭4byteがID,後々セッションIDとかにも使えるかな
    -payload:データ
-----
 *
 * */

fn build_udp_packet<'a>(
    buffer: &'a mut [u8],
    src_port: u16,
    dst_port: u16,
    chunk_id: u32,
    //checksum
    payload: &[u8],
) -> MutableUdpPacket<'a> {
    let mut udp_packet = MutableUdpPacket::new(buffer).expect("Failed to create UDP packet");
    udp_packet.set_source(src_port);
    udp_packet.set_destination(dst_port);
    udp_packet.set_length((8 + 4 + payload.len()) as u16);
    udp_packet.set_checksum(0); // Checksum can be computed here if needed
    let packet_payload = udp_packet.payload_mut();
    packet_payload[0..4].copy_from_slice(&chunk_id.to_be_bytes());
    packet_payload[4..4 + payload.len()].copy_from_slice(payload);
    udp_packet
}

fn build_ip_packet<'a>(
    buffer: &'a mut [u8],
    src: Ipv4Addr,
    dst: Ipv4Addr,
    payload_len: usize,
) -> MutableIpv4Packet<'a> {
    let mut ip_packet = MutableIpv4Packet::new(buffer).expect("Failed to create IP packet");
    ip_packet.set_version(4);
    ip_packet.set_header_length(5);
    ip_packet.set_total_length((20 + payload_len) as u16);
    ip_packet.set_next_level_protocol(IpNextHeaderProtocols::Udp);
    ip_packet.set_source(src);
    ip_packet.set_destination(dst);
    ip_packet.set_checksum(0); // Checksum can be computed here if needed
    ip_packet
}

fn main() {
    let src_ip = Ipv4Addr::new(111, 1, 1, 1);
    let dst_ip = Ipv4Addr::new(172, 21, 28, 6);
    let src_port = 1234;
    let dst_port = 1234;

    let file = File::open("test.jpg").expect("Failed to open file");
    let mut reader = BufReader::new(file);

    // Send the packet
    let (mut tx, _) = transport_channel(4096, TransportChannelType::Layer3(IpNextHeaderProtocols::Udp))
        .expect("Failed to create transport channel");

    let mut chunk_id = 0;
    loop{
        let mut buffer = vec![0u8; 1012];
        let bytes_read = reader.read(&mut buffer).expect("Faild read file");
        if bytes_read == 0 {
            break;
        }
        
        let mut udp_buffer = vec![0u8; 8 + 4 + bytes_read];

        //let udp_packet = build_udp_packet(&mut udp_buffer, src_port, dst_port, chunk_id, &buffer[..bytes_read]);
        //let udp_packet = build_udp_packet(&mut udp_buffer, src_port, dst_port, chunk_id, &buffer[..bytes_read]);
        build_udp_packet(&mut udp_buffer, src_port, dst_port, chunk_id, &buffer[..bytes_read]);
        //udp_packet.set_checksum(udp_packet);
        let mut ip_buffer = vec![0u8; 20 + udp_buffer.len()];
        let mut ip_packet = build_ip_packet(&mut ip_buffer, src_ip, dst_ip, udp_buffer.len());
        ip_packet.payload_mut().copy_from_slice(&udp_buffer);

        tx.send_to(ip_packet, dst_ip.into()).expect("Failed to send packet");
        println!("chunk {} sent", chunk_id);

        chunk_id += 1;
    }
    let mut udp_buffer = vec![0u8; 8 + 4];
    build_udp_packet(&mut udp_buffer, src_port, dst_port, END_SIG, &[]);

    let mut ip_buffer = vec![0u8; 20 + udp_buffer.len()];
    let mut ip_packet = build_ip_packet(&mut ip_buffer, src_ip, dst_ip, udp_buffer.len());

    ip_packet.payload_mut().copy_from_slice(&udp_buffer);
    tx.send_to(ip_packet, dst_ip.into()).expect("Failed to send packet");
    println!("complete");
}
