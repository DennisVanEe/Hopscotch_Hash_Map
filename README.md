# Hopscotch-Hash-Map

This hash map was developed for the KPCB Fellows application.

This hash map is based on the ideas presented in the 2008 paper titled Hopscotch Hashing (http://people.csail.mit.edu/shanir/publications/disc2008_submission_98.pdf)

The basic idea is that each item (bucket) in the hashmap is part of a neighborhood. So when a collision occurs, the item is simply moved to an empty space in the neighborhood. This would allow for more efficient look ups, as it would only have to look in the neighborhood. But, if a neighborhood is full, the map will "move" an empty space into the neighborhood by moving around items in such a way that they still belong to their respective neighborhood. 

Some notable modifications to the hashmap presented in the paper:
- No bitmap was used to represent a neighborhood, instead, naive lookup algorithm was used (a bitmap would require more memory and the lookup should be constant (neighborhood size), so as to still scale efficiently).
- The neighborhoods bases are centered instead of bounded solely to the right. This would allow neighborhoods near the ends of the map to still have the same size (as the neighborhood gets shifted from the center).
- A linked list linking items that do not fit into a neighborhood was used instead of resizing the hashmap. The hashmap has to be fixed size, so resizing was not an option. Instead, if an item can't be found in a neighborhood, it traverses a linked list in an attempt to find the item. These items technically belong to a neighborhood, but are marked otherwise so that they can freely be switched when moving empty spaces around. Looking up items in a linked list will be slower (not cache-friendly) but this should be rare, as a full bucket is (hopefully) rare. Depends on the hashing function utilized. 