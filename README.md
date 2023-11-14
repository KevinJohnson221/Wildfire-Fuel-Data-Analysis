# Wildfire-Fuel-Data-Analysis

Uses multiple threads to process data related to fuel types and data validation
Reads data from a text file using 10 fuel counting threads and 10 data validation threads to count the occurances and to check for invalid data.
Aggregates data after all threads have completed their tasks, calculates the total occurances of each fuel type across all sections, and prints the results, including if invalid data was found in the grid.
Threads synchronization used to wait for all threads to complete before aggregating the results.
Purpose was to create develop an efficent multi-threaded software tool that could analyze and process large datasets of numerical data.
