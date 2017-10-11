import javax.swing.*;        

public class RouterNode {
  private int myID;
  private GuiTextArea myGUI;
  private RouterSimulator sim;
  private int[] costs = new int[RouterSimulator.NUM_NODES];
  private int[][] dist_table = new int[RouterSimulator.NUM_NODES][RouterSimulator.NUM_NODES];
  private int[] next_hop = new int[RouterSimulator.NUM_NODES];

  private int do_poison = 1;
  // Dist table is of the form
  // From A:    to A   to B   to C
  //  via A
  //  via B
  //  via C
 	


  //--------------------------------------------------
  public RouterNode(int ID, RouterSimulator sim, int[] costs) {
    myID = ID;
    this.sim = sim;
    myGUI =new GuiTextArea("  Output window for Router #"+ ID + "  ");

    System.arraycopy(costs, 0, this.costs, 0, RouterSimulator.NUM_NODES);
    printDistanceTable();

    // Fill in distance table with 999
    for (int i=0; i < RouterSimulator.NUM_NODES; i++) {
        for (int j=0; j < RouterSimulator.NUM_NODES; j++) {
	dist_table[i][j] = 999;
        }
        next_hop[i] = -1;
    }
  
    // Set the value to known neightbours
    for (int i=0; i < RouterSimulator.NUM_NODES; i++) {
    	dist_table[i][i] = costs[i];
        next_hop[i] = i;
    }

    for (int i=0; i < RouterSimulator.NUM_NODES; i++) {
      if ( i != myID && dist_table[i][i] != 999) {
        RouterPacket new_packet = new RouterPacket(myID, i, costs);
        sendUpdate(new_packet);
      }
    }
  }

  //--------------------------------------------------
  public void recvUpdate(RouterPacket pkt) {
    // Receive new update:
    int receive_node = pkt.sourceid;

    // Update dist table
    for (int i=0; i < RouterSimulator.NUM_NODES; i++) {
    	dist_table[receive_node][i] = dist_table[receive_node][receive_node] + pkt.mincost[i];
    }

    // Find new min dist for each node
    int table_changed = 0;

    for (int i=0; i < RouterSimulator.NUM_NODES; i++) {
        if ( i != myID) {
            // Find min dist
            int min_dist = 999;
            int min_dist_dir = -1;

            // Check each neighbour
            for (int j=0; j < RouterSimulator.NUM_NODES; j++) {
	      if ( j != myID && dist_table[j][i] != 999) {
	          if(dist_table[j][i] < min_dist) {
		    min_dist = dist_table[j][i];
                    min_dist_dir = j;
                  }
              }
            }
            if (costs[i] != min_dist) {
    	      costs[i] = min_dist;
              next_hop[i] = min_dist_dir;
              table_changed = 1;
            }
        }
    }

    // Update the List  
    if (table_changed > 0 ) {
    for (int i=0; i < RouterSimulator.NUM_NODES; i++) {
      if ( i != myID && dist_table[i][i] != 999) {
        int[] costs_to_send = new int[RouterSimulator.NUM_NODES];
        System.arraycopy(costs, 0, costs_to_send, 0, RouterSimulator.NUM_NODES);

        if (do_poison > 0) {
	  for (int j= 0; j < RouterSimulator.NUM_NODES; j++) {
            if (next_hop[j] == i) {
              costs_to_send[j] = 999;
            }
          }
        }
        RouterPacket new_packet = new RouterPacket(myID, i, costs_to_send);
        sendUpdate(new_packet);
      }
    }
    }
  }
  

  //--------------------------------------------------
  private void sendUpdate(RouterPacket pkt) {
    sim.toLayer2(pkt);

  }
  

  //--------------------------------------------------
  public void printDistanceTable() {
	  myGUI.println("Current table for " + myID +
			"  at time " + sim.getClocktime());
 
          myGUI.println("Distancetable:");
          myGUI.println_sameline("dst    |    ");
          for (int k=0; k<RouterSimulator.NUM_NODES; ++k)
             myGUI.println_sameline(k + "\t");
          myGUI.println_sameline("\n---------------------------------");
          myGUI.println("---------------------------------");
          
          for (int i=0; i<RouterSimulator.NUM_NODES; i++) {
             myGUI.println_sameline("nbr " + i + " |   ");
             for (int j=0; j<RouterSimulator.NUM_NODES; j++) {
                //print lines, so we want a fix 'i' and loop through 'j'
                myGUI.println_sameline(dist_table[i][j] + "\t");
             }
             myGUI.println();
          }
  }

  //--------------------------------------------------
  public void updateLinkCost(int dest, int newcost) {
    // Update all entries through dest
    int oldcost = dist_table[dest][dest];


    if (oldcost == 999) {
	dist_table[dest][dest] =  newcost;
    } else {
	for (int i=0; i < RouterSimulator.NUM_NODES; i++) {
	      if ( i != myID) {
                 if (newcost == 999) {
                     dist_table[dest][i] = 999;
                 } else {
	             dist_table[dest][i] = dist_table[dest][i] - oldcost + newcost;
                 }
              }
        }
    }


    // Find new min dist for each node
    int table_changed = 0;

    for (int i=0; i < RouterSimulator.NUM_NODES; i++) {
        if ( i != myID) {
            // Find min dist
            int min_dist = 999;
            int min_dist_dir = -1;

            // Check each neighbour
            for (int j=0; j < RouterSimulator.NUM_NODES; j++) {
	      if ( j != myID && dist_table[j][i] != 999) {
	          if(dist_table[j][i] < min_dist) {
		    min_dist = dist_table[j][i];
                    min_dist_dir = j;
                  }
              }
            }
            if (costs[i] != min_dist) {
    	      costs[i] = min_dist;
              table_changed = 1;
              next_hop[i] = min_dist_dir;
            }
        }
    }

    // Update the List  (TODO check if something changed)

    if (table_changed > 0 ) {
    for (int i=0; i < RouterSimulator.NUM_NODES; i++) {
      if ( i != myID && dist_table[i][i] != 999) {
        int[] costs_to_send = new int[RouterSimulator.NUM_NODES];
        System.arraycopy(costs, 0, costs_to_send, 0, RouterSimulator.NUM_NODES);

        if (do_poison > 0) {
	  for (int j= 0; j < RouterSimulator.NUM_NODES; j++) {
            if (next_hop[j] == i) {
              costs_to_send[j] = 999;
            }
          }
        }

        RouterPacket new_packet = new RouterPacket(myID, i, costs_to_send);
        sendUpdate(new_packet);
      }
    }
    }

  }

}
