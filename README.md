# Route Lookup
*Authors: María Sanz Gómez & Gonzalo Guerrero Torija.*

Route lookup code implemented by using a two-level hardware multi-bit trie to achieve faster results than a linear search. Developed as a part of the evaluation of the course Switching 2020-21 at UC3M.

To compile the code, in the folder directory, just: 
```
~$: make clean
~$: make
```
The program takes 2 arguments. The first one can be either `routing_table_simple.txt` or `routing_table.txt`. It is all the entries that the router will store in hardware. The second one can be either `prueba0.txt`, `prueba1.txt`, `prueba2.txt` or `prueba3.txt`. These files contain diverse IP routes and are used to check the correct performance of the code. The code generates an output file with the IPs from the second argument and the output socket decided by the algorithm. To run it:

```
~$: ./my_route_lookup routing_table.txt prueba3.txt
```
