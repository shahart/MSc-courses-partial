import javax.swing.JApplet;
import java.awt.*;

public class TrafficLights extends JApplet implements Runnable {

	private final int COLORS=3;
	private final int LIGHTS=4;
	
	private Image m_image=null;
	private Graphics m_g;
	
	private Thread timer=null;
	
	private int[][] durations=new int[LIGHTS][];
	private int[] light=new int[LIGHTS],past=new int[LIGHTS];
	private int walk=0;

	public void start()
	{
		if (timer==null) {
			
		   durations[0]=new int [COLORS];

		   if (getParameter("red")!=null)
		      durations[0][0]=Integer.parseInt(getParameter("red"));
		   else
		      durations[0][0]=10;
		      
		   if (getParameter("yellow")!=null)
			  durations[0][1]=Integer.parseInt(getParameter("yellow"));
		   else
			  durations[0][1]=20;
			  
		   if (getParameter("green")!=null)
			  durations[0][2]=Integer.parseInt(getParameter("green"));
		   else
			  durations[0][2]=30;

 	       durations[2]=durations[0];

		   durations[1]=new int [COLORS];
		   
		   durations[1][0]=durations[0][2];
		   durations[1][1]=durations[0][1];
		   durations[1][2]=durations[2][0];

		   durations[3]=durations[1];

		   light[0]=0;          // red 
		   light[2]=light[0];   // רמזור נגדי
		   light[1]=2;          // green - מימין ל"אדום
		   light[3]=light[1];   // נגדי ל-1 ולכן זהה

		   timer=new Thread(this);
		   timer.start();
		}
	}

	public void update(Graphics g)
	{
	   paint(g);
	}
  
	public void paint(Graphics g)
	{
	   int i;
	   
	   if (m_image==null) {
		  m_image=createImage(400,130);
		  // make a double buffer
		  m_g=m_image.getGraphics();
		  // ציור רמזור רכב
		  m_g.setColor(Color.gray);
		  for (i=0; i<LIGHTS; i++)
			 m_g.fillRect(100*i,0,80,100);
		  // רמזור הולכי רגל
		  m_g.setColor(Color.black);
		  for (i=0; i<LIGHTS; i++)
		     m_g.drawRect(34+100*i,110,11,21);
	   }

       for (i=0; i<LIGHTS; i++) {
		  Color[] shows={Color.red,Color.orange,Color.green};
       	  // מחק אור רכב קודם
		  m_g.setColor(Color.gray);
		  for (int j=0; j<COLORS; j++)
			 m_g.fillOval(30+100*i,5+35*j,20,20);
          // ציור אור נוכחי
		  m_g.setColor(shows[light[i]]);
		  m_g.fillOval(30+100*i,5+35*light[i],20,20);
          // מחק אור הולכי רגל
		  m_g.setColor(getBackground());
		  m_g.fillRect(35+100*i,120,10,10);
		  m_g.fillRect(35+100*i,110,10,10);
          // ציור אור הולכי רגל
		  if (light[i]!=0) {
		     m_g.setColor(Color.red);
  		     m_g.fillRect(35+100*i,110,10,10);
		  }
		  else if (walk==0) {
			 m_g.setColor(Color.green);
		     m_g.fillRect(35+100*i,120,10,10);
		  }
	   }
	   g.drawImage(m_image,0,0,null);
	}

	public void run()
	{
	   while (timer!=null) {
		  walk=1-walk;
		  for (int i=0; i<LIGHTS; i++) {
			 past[i]++;
			 // האם הרמזור דלוק לפי הזמן הקצוב לו ?
			 // והאם אחד משאר האורות בוטל ?
			 for (int j=0; j<COLORS && past[i]>=durations[i][light[i]]; j++) {
				past[i]=0;
				light[i]=(light[i]+1)%COLORS;
			 }
		  }
		  try { Thread.sleep(100); }
		  catch (InterruptedException e) {}
		  repaint(0,0,400,130);
	   }
	}
}
