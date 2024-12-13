# System Programming Lab 11 Multiprocessing
| Process | Threads 6 | Threads 8 | Threads 16 | Threads 18 | Threads 20 |
|---------|-----------|-----------|------------|------------|------------|
| 6       | 7.095     | 8.256     | 8.16       | 8.433      | 8.068      |
| 8       | 7.258     | 8.885     | 8.121      | 7.669      | 8.568      |
| 16      | 7.456     | 8.609     | 8.559      | 8.865      | 7.496      |
| 18      | 6.781     | 6.07      | 6.112      | 8.391      | 7.035      |
| 20      | 6.485     | 6.747     | 6.48       | 6.045      | 6.17       |

i. Multiprocessing seems to impact runtime more significantly than multithreading, likely due to its ability to leverage multiple CPU cores 

ii. The best runtime appears to occur with 20 processes and 18 threads 
