use pnet::packet::ip::IpNextHeaderProtocols;
use pnet::packet::udp::{MutableUdpPacket};
use pnet::packet::ipv4::MutableIpv4Packet;
use pnet::packet::MutablePacket;
use pnet::transport::{transport_channel, TransportChannelType};
use std::net::{Ipv4Addr};

const 

fn build_udp_packet<'a>(
    buffer: &'a mut [u8],
    src_port: u16,
    dst_port: u16,
    chunk: u8,
    payload: &[u64],
) -> MutableUdpPacket<'a> {
    let mut udp_packet = MutableUdpPacket::new(buffer).expect("Failed to create UDP packet");
    udp_packet.set_source(src_port);
    udp_packet.set_destination(dst_port);
    udp_packet.set_length((8 + payload.len()) as u16);
    udp_packet.set_checksum(0); // Checksum can be computed here if needed
    udp_packet.payload_mut().copy_from_slice(payload);
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
    let src_ip = Ipv4Addr::new(127, 0, 0, 1);
    let dst_ip = Ipv4Addr::new(127, 0, 0, 1);
    let src_port = 1234;
    let dst_port = 12345;

    let payload = b"Hello, World!";
    let mut udp_buffer = vec![0u8; 8 + payload.len()];
    let udp_packet = build_udp_packet(&mut udp_buffer, src_port, dst_port, payload);

    let mut ip_buffer = vec![0u8; 20 + udp_buffer.len()];
    let mut ip_packet = build_ip_packet(&mut ip_buffer, src_ip, dst_ip, udp_buffer.len());

    // Combine IP header and UDP packet
    ip_packet.payload_mut().copy_from_slice(&udp_buffer);

    // Send the packet
    let (mut tx, _) = transport_channel(4096, TransportChannelType::Layer3(IpNextHeaderProtocols::Udp))
        .expect("Failed to create transport channel");

    FILE f = fopen("img.png", "rb");
    chunk = 0;
    while(){
        fgets(buffer, sizeof(buffer), f);
        build_udppaket();
        tx.send_to(ip_packet, dst_ip.into()).expect("Failed to send packet");
        chunk += 1;
    }
    chunk = END_SIG;
}

