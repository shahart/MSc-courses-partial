import javax.swing.*;
import java.io.*;
import java.net.*;
import java.awt.event.*;

public class WeatherServer extends JApplet {
	
	private String citiesNames[]={"paris", "tel-aviv", "cairo"};
	private double temps[]=new double[citiesNames.length];
	private JComboBox cities=new JComboBox(citiesNames);

    private DatagramSocket socket=null;
    private boolean moreClients=true;
    
	private class Handler implements ActionListener {
		public void actionPerformed(ActionEvent e) {
			int index=cities.getSelectedIndex();
			String temp=JOptionPane.showInputDialog(temps[index]+" Celsius in "+citiesNames[index]+". New temp:");
			
			if (!(temp instanceof String) || temp.length()==0) 		// cancel זיהוי 
			   return;
			   
			temps[index]=Double.parseDouble(temp);
			   
			if (temps[index]==0) 
			   moreClients=false;
		}
	}

    public void start()
    {
    	DatagramPacket packet;
    	while (moreClients) {
    		try {
    			byte[] buf=new byte[256];
    			
    			// receive request
    			packet=new DatagramPacket(buf,buf.length);
    			socket.receive(packet);

				String received=new String(packet.getData()).substring(0,packet.getLength());
				
				int index=0;
				while (index<citiesNames.length && !received.equalsIgnoreCase(citiesNames[index]))
				   index++;
    			
    			// prepare response
    			String tempString;
    			if (index<citiesNames.length) 
    			   tempString=new Double(temps[index]).toString();
    			else
    			   tempString=new String("city not found");
    			
    			// send the response to "address" and "port"
    			packet=new DatagramPacket(tempString.getBytes(),tempString.length(),packet.getAddress(),packet.getPort());
    			socket.send(packet);
    		}
    		catch (IOException e) {	moreClients=false; }
    	}
    	socket.close();
    }
	
	public void init()
	{
		cities.addActionListener(new Handler());
		getContentPane().add(cities);
		setSize( 100, 70 );
		setVisible( true );
		
		try { socket=new DatagramSocket(1800); }
		catch (SocketException e) {	}
	}
}
