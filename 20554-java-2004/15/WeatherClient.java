import java.io.*;
import java.net.*;

public class WeatherClient {

	public static void main(String[] args) throws IOException {
		if (args.length<3) {
		   System.out.println("used paramters: host port city");
		   return;
		}
		 
		// get a datagram socket on any available port
		try {
			DatagramSocket socket=new DatagramSocket();
			
			// send request
			byte[] buf=new byte[256];
			InetAddress addr=InetAddress.getByName(args[0]);
			DatagramPacket packet=new DatagramPacket(args[2].getBytes(),args[2].length(),addr,Integer.parseInt(args[1]));
			socket.send(packet);
			
			// get response
			packet=new DatagramPacket(buf,buf.length);
			socket.receive(packet);
			
			// display response
			String received=new String(packet.getData()).substring(0,packet.getLength());
			System.out.println(received);
			socket.close();
		}
		catch (SocketException e) { System.out.println("check server"); }
	    catch (UnknownHostException e) { System.out.println("host not found"); }
	    catch (IOException e) {};	    
	}
}
