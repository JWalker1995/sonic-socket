var max_exponent = 200;
var num_fields = 100;

// End config


var assert = require('assert');
var fs = require('fs');
var child_process = require('child_process');

var first_decrement = 2;
var decrements = [1,1,3,1,3,1,5,3,3,9,3,1,3,19,15,1,5,1,3,9,3,15,3,39,5,39,57,3,35,1,5,9,41,31,5,25,45,7,87,21,11,57,17,55,21,115,59,81,27,129,47,111,33,55,5,13,27,55,93,1,57,25,59,49,5,19,23,19,35,231,93,69,35,97,15,33,11,67,65,51,57,55,35,19,35,67,299,1,33,45,83,25,3,15,17,141,51,115,15,69,33,97,17,13,117,1,59,31,21,37,75,133,11,67,3,279,5,69,119,73,3,67,59,9,137,1,159,25,5,69,347,99,45,45,113,13,105,187,27,9,111,69,83,151,153,145,167,31,3,195,17,69,243,31,143,19,15,91,47,159,101,55,63,25,5,135,257,643,143,19,95,55,3,229,233,339,41,49,47,165,161,147,33,303,371,85,125,25,11,19,237,31,33,135,15,75,17,49,75,55,183,159,167,81,5,91,299,33,47,175,23,3,185,157,377,61,33,121,77,3,117,235,63,49,5,405,93,91,27,165,567,3,83,15,209,181,161,87,467,39,63,9,189,163,107,81,237,75,207,9,129,273,245,19,189,93,87,361,149,223,71,747,275,49,3,265,77,241,53,169,237,205,305,129,89,103,93,69,47,139,83,45,173,9,165,115,167,493,47,19,167,601,35,171,285,123,341,69,153,265,267,121,75,103,503,99,159,493,77,45,203,139,113,465,57,33,165,795,197,9,11,141,23,399,101,595,155,139,255,61,707,483,243,321,3,75,15,147,293,229,65,199,119,475,45,211,117,285,113,61,657,139,153,49,173,243,671,411,719,369,605,75,923,169,167,487,315,25,495,741,177,333,65,679,57,259,417,19,65,313,105,31,317,265,231,615,45,21,137,105,107,93,377,531,605,81,131,91,593,31,53,379,257,235,923,157,1005,103,51,205,183,21,17,45,435,1029,147,69,317,271,101,91,389,301,321,781,65,55,677,201,299,601,183,97,87,93,417,151,33,361,995,685,17,195,77,325,203,241,501,2239,3,285,57,217,627,273,57,549,77,649,195,157,437,819,813,511,17,283,35,147,209,579,135,129,153,123,95,537,47,273,275,301,39,1399,143,57,17,21,917,75,129,433,125,31,257,1291,563,151,863,45,671,91,503,91,45,265,243,741,75,187,569,445,767,225,689,489,917,91,383,1,117,1267,465,363,363,1017,65,31,1017,315,335,153,521,367,149,9,53,615,335,81,611,157,759,3,11,537,749,625,5,231,503,441,567,595,1295,339,261,411,717,153,743,9,95,45,113,297,645,199,261,369,275,193,105,1057,789,81,483,91,57,295,231,27,195,81,201,285,513,181,773,291,935,363,165,81,117,591,587,91,95,229,75,1137,585,519,153,1,527,1125,27,187,683,381,297,201,459,199,2487,45,15,223,195,219,117,199,131,217,455,709,395,759,813,441,141,135,87,111,405,499,305,73,563,397,369,1731,987,357,195,123,611,565,333,475,971,405,143,723,785,799,473,589,1017,67,17,541,93,241,209,339,161,1185,399,141,87,517,767,631,41,1959,939,159,63,45,107,679,651,871,759,3,455,1057,65,739,3,49,447,343,723,9,1113,159,87,529,245,1255,5,91,149,1923,1127,207,489,729,167,7,153,25,753,717,395,61,275,465,287,33,807,657,77,9,483,951,303,609,261,607,509,771,171,789,875,795,1281,37,173,225,665,1299,153,723,161,165,875,1101,417,199,107,141,363,529,173,285,143,267,393,415,641,1081,825,279,495,67,545,649,611,381,1539,1011,483,87,147,3345,551,301,759,349,137,277,453,201,677,337,1299,175,513,409,267,1095,503,411,105,205,47,495,387,511,621,189,17,313,5,87,195,73,281,61,959,295,281,579,519,139,195,97,209,61,755,199,423,123,153,1869,143,519,87,1821,447,1335,95,639,213,201,371,445,435,301,63,399,17,871,3,3417,777,1923,393,387,459,441,437,825,1967,441,1887,421,243,103,1713,555,467,21,783,301,177,441,1065,727,117,393,111,681,113,661,2201,91,189,541,45,69,915,49,135,201,395,1065,881,481,213,649,365,337,207,1281,1667,585,609,75,353,49,423,153,357,1191,1935,981,185,2145,683,189,957,241,185,669,483,31,143,421,1037,1501,645,979,765,129,617,133,365,799,1325,489,1601,205,215,385,627,367,573,451,1055,519,1629,81,117,1467,99,229,485,1027,1337,489,1127,439,167,129,1697,109,293,3151,1383,69,1347,1179,237,547,185,1153,651,115,2147,169,2355,819,525,325,1631,405,557,805,77,927,273,243,195,537,1779,45,1133,987,123,103,1133,139,1245,549,387,241,1037,979,825,61,33,1131,231,1209,87,1725,347,789,303,1449,57,135,537,553,755,361,105,2673,2531,4959,497,1195,27,171,909,319,897,441,1127,1359,1535,75,395,1083,1065,307,567,1435,101,1095,449,243,1881,175,3789,313,33,381,725,603,125,1449,599,349,177,585,257,51,113,525,279,195,783,741,1305,111,683,2301,825,33,801,75,405,235,1223,207,285,1321,3323,4005,89,849,53,1455,1085,255,311,45,2255,411,263,1035,75,303,173,55,1049,1335,483,199,405,363,413,325,129,1129,587,421,93,703,533,795,2555,481,1727,915,2627,195,605,2259,627,855,257,597,39,1015,83,957,5,609,185,1729,185,1855,1235,1095,1079,303,131,3181,617,549,237,139,927,999,1121,345,293,1629,681,129,2523,205,1077,117,333,201,1181,279,1013,1281,833,49,63,1041,195,229,335,211,825,885,113,55,1605,69,107,405,243,1455,483,133,827,181,503,729,713,561,299,111,227,979,305,151,1371,741,35,85,1283,357,237,63,405,987,249,589,65,679,563,849,1673,525,227,469,1265,421,1577,369,2247,219,5,181,123,511,267,471,405,715,165,129,65,55,669,73,1535,717,1139,675,165,331,635,265,597,1237,495,1065,453,67,233,1861,405,645,35,465,941,31,299,3705,2393,181,257,1159,1617,1095,1593,735,825,691,117,555,45,1,1175,1339,1793,691,773,529,261,387,1487,1489,425,325,3023,819,587,817,767,1743,741,2007,75,1323,1301,1677,927,871,347,757,243,1993,201,1609,509,1015,971,1225,723,453,1187,769,377,1195,141,661,209,1521,173,1495,807,979,477,451,1623,699,2091,2407,1029,415,423,865,2397,1945,227,1351,1175,81,1151,441,467,349,291,295,317,2781,141,495,87,1179,3947,459,747,99,425,1081,903,363,1377,547,1187,645,117,291,273,421,383,291,929,319,747,561,719,1491,435,1459,3153,2101,483,1879,1479,1515,1331,559,33,609,1155,355,903,615,1073,439,609,741,467,61,767,963,1415,285,413,55,41,1999,395,513,893,2277,117,445,257,555,1287,411,611,381,575,2055,1085,385,983,1519,611,705,339,589,23,885,173,1843,1251,997,857,253,561,121,2373,255,177,805,65,651,2111,4147,1217,489,1125,357,827,2869,437,939,153,363,533,685,297,1083,2067,55,573,705,593,759,5309,441,795,289,975,1795,885,195,717,315,75,417,3317,1845,35,421,189,1119,1335,559,95,363,677,237,569,1165,1293,211,669,2143,815,325,1019,33,257,2805,2633,4533,2525,2031,263,963,371,471,1739,135,2063,775,509,171,4551,45,705,1951,51,49,155,2491,1203,225,315,73,1893,1317,3453,175,651,235,417,195,1371,1309,617,1081,2747,831,1095,571,3317,2617,1397,4381,485,1711,245,1441,2661,687,1323,1999,1097,409,587,135,1655,307,1137,403,3185,1101,737,45,1233,669,125,1555,585,57,129,415,2355,211,557,1111,107,1077,615,21,9903,3015,23,1105,2147,1057,965,2203,231,745,2273,765,797,3379,1797,1161,2457,399,107,339,267,1119,3015,1525,2307,1609,723,1405,1361,1461,1875,109,3263,4347,537,1909,3335,727,2057,63,1151,861,605,91,561,531,2775,1083,65,1627,387,885,315,1149,219,3561,15,1677,1049,163,2153,4975,3429,2013,437,987,2183,309,887,1389,675,6583,1491,1237,1233,469,525,495,23,153,1331,1221,4469,2679,1575,777,1265,181,1685,1797,759,1291,177,165,1187,343,171,741,3173,735,161,259,1983,901,911,877,1113,1689,33,871,395,2263,1163,717,635,1063,1877,949,1137,1099,747,2085,3885,21,3587,1881,359,475,243,1525,977,1351,485,669,539,61,573,1309,1115,999,1851,319,69,399,837,2055,3,2001,3755,57,5925,271,723,637,2519,1581,543,1719,693,1635,717,1297,167,391,801,889,1533,541,497,937,2123,1339,1877,511,4359,3595,2207,171,537,1521,1151,2097,563,975,711,1137,1073,175,495,1885,197,2439,4781,771,297,1995,1751,651,279,2623,975,597,963,603,65,721,885,961,323,1855,2163,2649,371,321,1085,771,2243,2359,3009,405,761,1381,2163,1551,2505,759,2439,2259,5,207,663,625,2747,2485,6723,685,2805,1257,615,891,3705,2379,4683,61,873,1065,629,2233,657,3259,455,163,663,2425,2123,4821,2747,2169,195,309,15,217,1239,579,1155,579,1767,2149,3287,271,119,439,3881,979,119,979,567,511,1965,2401,57,457,1889,1063,87,79,2075,829,521,1681,119,2259,1593,285,1137,4189,2687,181,129,2241,1025,15,1067,4293,425,1527,1685,559,4053,1915,395,675,1161,631,99,783,573,2527,633,1011,1175,987,1149,315,963,2239,1587,223,1523,537,1503,2443,2417,441,7085,3625,1361,175,1473,1441,707,49,219,775,741,181,615,85,5627,55,4515,1453,797,609,1005,1071,1133,769,2747,199,1691,847,3479,661,111,1167,275,1249,815,2581,129,2401,1641,519,1653,1153,41,2935,303,1831,3531,1561,45,2379,5465,517,393,319,2003,1707,3645,4101,513,7,815,841,1067,2115,605,2335,1151,2227,189,739,2127,147,2397,583,485,2131,2217,921,297,4381,213,235,57,2265,797,1965,495,3211,363,699,2531,1087,5133,2359,495,1231,69,229,53,985,537,1125,1553,1755,2825,2185,1085,1995,567,69,3927,217,567,1993,963,805,359,921,1011,549,63,301,633,85,1557,1449,7323,289,395,2211,675,321,1037,505,203,1345,1863,511,315,1479,2307,129,4443,1131,1617,3241,611,5059,863,735,2723,117,465,453,593,1837,3339,75,5105,5031,1037,3441,833,2275,537,465,215,199,3947,603,2331,537,1679,1021,1035,3345,783,1491,3561,4797,2289,1911,2247,1407,4137,4141,363,187,887,2283,2003,6081,293,475,1937,2869,1193,1251,3003,5097,1115,4375,93,849,2337,429,161,199,999,7465,1175,2625,1305,175,1515,2271,1377,345,803,861,215,429,3053,5241,759,1713,6743,1255,1767,1569,2021,97,153,483,3473,619,1763,4659,233,1615,2055,229,773,129,2715,51,3381,597,387,651,2715,2149,1833,373,801,4107,3813,663,1457,309,4949,7951,1907,507,249,381,1763,771,4839,1003,165,1975,789,1945,1311,2299,153,283,1763,1,2295,39,2681,69,7559,1483,761,2511,369,1003,1451,619,3159,1749,711,3151,1775,735,3143,1869,867,141,5367,1971,359,2205,563,1605,597,649,1533,1467,873,1869,2175,2079,99,1171,1497,831,1329,1483,227,2125,1805,289,1631,201,615,3051,867,1329,1689,1593,437,937,4323,259,497,109,869,2785,1803,499,2103,201,267,2511,845,4623,861,619,567,2095,171,4539,1329,1,117,1509,129,771,1085,2131,1545,201,363,415,2313,2419,3585,301,2465,3621,2175,1057,743,1525,747,2667,1857,1863,2663,1887,447,271,2265,1959,743,429,2213,5671,1157,1615,3561,7,4103,3,405,2245,1007,579,1235,2145,65,85,3533,3385,1893,121,2613,565,99,4125,771,1479,2187,703,911,3009,87,723,1271,5611,623,1893,1763,37,237,109,1437,415,6879,8545,983,91,5145,691,2517,435,395,481,533,3721,5,3703,41,3931,1779,105,4817,1279,3447,771,87,51,519,169,3597,9205,999,273,927,517,1409,1299,7353,1101,893,673,411,3759,303,193,1383,429,5,579,75,795,1625,213,6783,1959,309,165,1023,2245,3353,5841,317,181,8679,391,1287,649,1547,349,525,495,945,141,1491,445,875,1263,905,637,3723,2565,1953,2515,2687,1209,1965,2757,4355,531,2603,2415,3849,4771,333,2305,347,241,1095,37,467,4363,567,339,1997,45,315,1765,2699,115,327,511,129,759,1127,2197,987,2665,167,997,3815,4059,1307,429,129,771,681,2407,317,2131,6405,31,7103,2539,443,585,507,183,143,2545,2955,2425,1131,831,257,1245,353,7,1623,1183,1157,1627,285,451,1191,2311,1599,3115,3537,679,377,1885,413,777,1353,501,1655,7281,653,3855,3083,979,2427,1645,765,27,869,991,381,231,2313,2509,243,2851,12053,7183,243,547,293,1255,155,769,1937,1645,717,91,2777,6231,3153,55,285,2169,455,1399,195,369,681,709,75,615,2961,1357,239,861,497,5775,177,45,351,3295,867,1623,245,459,4499,3591,155,1357,77,3153,3047,1257,2229,1465,1545,607,2057,121,5645,6721,2235,1851,1533,601,2397,8893,651,3675,999,5025,705,5287,1209,2373,231,409,1343,973,1095,1711,3767,963,327,1071,107,1639,1553,975,405,4275,401,2301,149,375,1287,801,639,313,2117,2937,1713,135,167,771,1979,255,71,2175,1995,693,833,949,1263,343,1323,5325,569,355,383,2365,1547,5451,3827,1161,437,4483,833,4291,387,3091,383,1231,2985,979,87,387,1083,2325,2253,7995,89,3643,315,181,45,2175,2607,531,7739,2383,2021,2535,395,513,2555,3111,2529,741,933,1287,669,4035,123,477,5129,2895,2385,1305,773,1425,425,277,227,2731,1487,559,653,4353,525,1287,143,1671,315,1927,2555,2113,2253,5181,1869,865,1293,1269,345,2469,5375,1117,1149,2395,4223,4159,2213,409,2061,31,2175,1449,6657,6051,875,1903,2421,2337,297,1921,545,301,2763,3091,275,3051,2693,2641,4791,7071,149,2005,197,2047,1955,1621,4641,5697,1203,3199,191,4521,1253,55,1991,3109,353,1525,3285,361,213,8805,717,2251,3645,5295,333,3877,4619,141,435,769,3869,1113,2231,3325,6069,313,1385,1585,749,181,1623,81,909,391,1745,145,843,3219,4845,127,5885,2689,4613,1909,533,931,983,1041,2247,55,743,1797,213,183,3921,3157,2103,1045,807,1447,675,1231,4611,21,173,139,117,6529,195,2743,3377,1347,3905,9,63,429,10187,5941,107,1287,1779,1573,951,1855,419,3919,3375,327,753,351,41,471,137,1483,515,801,1337,1945,371,3057,249,979,1637,5235,1659,2721,161,4969,6245,1479,171,799,2499,411,525,3835,147,765,291,6451,2877,5929,305,327,363,2029,1821,1621,1235,313,3135,2409,159,273,791,6075,1493,4623,3365,2307,819,255,1991,1881,3947,1129,1341,157,227,5383,1073,2091,669,1011,4155,3009,1695,1053,1457,1485,2439,711,275,1879,1097,1711,1757,5377,899,4041,4103,289,2309,1015,197,471,89,451,873,727,1617,1575,105,477,63,3909,5475,8439,1625,2155,6893,1147,369,1609,4245,247,1217,6549,2667,361,4383,3331,4953,4291,1077,1389,2033,385,1407,861,1701,129,195,1755,837,279,1125,765,5057,1141,1799,475,3507,889,6237,841,8837,1845,677,8229,2661,1417,2699,2113,3443,397,1517,205,717,1509,3057,789,785,2527,4443,399,57,805,167,1993,303,1369,1539,481,1131,645,3933,1665,923,1909,105,1531,483,2449,2429,3663,4275,145,1697,759,813,777,6287,385,3875,3741,455,361,2471,979,3603,573,2621,5361,1653,5073,2841,4675,57,1423,5237,2229,107,2091,231,1221,2987,1863,6951,5997,1575,3591,237,187,47,841,521,1687,323,855,2103,25,189,241,573,717,5793,3513,951,25,3473,3411,2525,6739,539,501,143,489,5319,2413,8697,3511,2609,4089,3087,175,587,289,1341,289,2477,1621,603,1495,875,3939,6801,3379,3149,4345,885,867,75,1701,111,1959,2433,75,2415,889,5,159,1215,5797,3057,73,1191,2589,2507,1093,2583,2931,1083,703,5085,1569,1035,861,1463,2665,173,6685,4725,2887,4563,3085,581,729,2609,2083,2813,675,875,831,401,1455,629,2079,237,351,1227,445,1191,1561,4299,495,237,1345,203,2403,143,577,1955,7143,6665,4357,1007,1273,501,7321,315,145,363,831,1725,1291,1881,4405,147,2583,5907,187,1683,1089,263,2569,165,189,657,1005,5319,13531,917,4285,377,61,2817,1557,2469,1,75,969,9609,853,1805,1209,1373,8209,221,4009,593,13,2303,4059,755,9451,257,1875,1349,3,1457,1867,863,2275,203,891,2085,1183,893,501,495,3421,5855,351,1619,1215,2691,1951,4119,433,921,577,2495,1995,2475,2025,1703,7375,267,2901,743,14443,231,6051,2507,1479,3057,3151,749,5809,4445,231,1635,639,2165,475,2309,991,1295,1219,2163,525,3281,765,2427,2331,5427,2371,2729,1665,1967,3781,777,1821,1233,1171,99,4359,347,4591,4137,3199,5813,771,839,43,2691,121,2285,3895,681,3037,4587,2491,953,139,1955,1513,3225,585,2639,661,4007,199,843,3765,2027,531,8507,4051,23,1245,425,31,2247,237,4287,1105,1533,1557,4475,3805,975,675,3479,1023,1005,2139,2465,2881,1013,1951,4085,711,1221,3615,1307,9,2037,3867,4185,3015,2757,765,1479,3513,363,985,945,621,4137,559,2673,2509,483,3769,1299,313,335,2121,195,1219,3761,1627,1079,1131,2861,6187,3033,2343,6893,537,5433,3393,1865,4359,857,1899,203,1405,1649,183,2463,1969,1769,901,4871,295,6645,4339,2187,1965,689,1089,4377,895,2409,2311,2901,1795,2603,6285,357,10257,3237,2595,1515,2565,3027,1249,447,55,549,933,2513,91,1265,2335,257,2221,4155,2829,423,691,2285,973,1055,1401,695,2341,4551,1209,2027,663,1215,2875,6503,5679,1251,2931,545,1431,765,3595,1167,1963,1427,759,1629,3231,4845,3997,1487,1513,2043,829,6843,1309,1097,705,5585,33,227,5379,45,1485,2541,199,6663,3655,681,1417,1697,853,1047,435,1997,2341,4677,1237,417,1125,2505,1305,1407,3661,917,495,1635,1609,5925,1509,6063,645,4253,2145,4245,493,393,387,567,7459,1175,361,2465,8395,1563,14859,203,1003,77,2695,1599,2209,7217,691,1865,2751,5301,6951,7643,1323,227,177,797,1741,1097,951,3317,3799,3185,1089,2669,3703,473,5979,2207,1461,5801,10615,459,2119,1323,3745,677,3135,1161,1555,7817,1581,423,3159,1103,2085,1277,3097,429,411,1911,181,1635,3403,1443,6169,4967,8541,5315,2547,8597,985,1761,157,1193,2713,1821,1287,6359,1021,3513,3525,1139,4335,563,4209,3675,3085,983,2527,3927,9169,3035,2659,4877,403,6005,159,3015,8905,827,529,1473,2241,495,489,17,579,4721,1035,1083,589,1575,1795,1749,3099,1397,2061,2565,405,3305,1819,1335,885,4275,2739,515,1521,825,6919,1457,49,1067,6469,1547,693,1727,795,1107,5005,5247,3165,609,19,2565,8055,17105,1639,10311,2547,9569,495,75,327,135,1789,665,51,1763,133,645,1251,5087,909,4301,2197,3125,121,767,7629,3015,3345,8807,829,2015,1263,525,1555,1115,123,255,955,2859,7501,3513,2067,3449,5955,4455,3427,15465,1543,7713,2535,555,1249,8037,8901,3633,1513,861,1039,917,1215,281,801,117,3543,7031,3231,285,1303,1011,6321,317,3063,4355,2485,845,5011,345,759,3233,2245,531,439,6135,2151,963,555,675,4035,83,1761,11009,225,143,111,839,2775,1685,1797,6653,2271,2585,2727,5705,549,4463,7,2753,14499,495,2421,3219,1495,4007,835,3233,8641,1181,327,149,405,1071,667,11637,229,1595,465,3437,639,3465,3759,3729,1473,731,919,353,4429,567,1959,1439,2971,965,2527,1695,2883,6971,2725,3983,6549,1623,2449,899,2869,1683,175,3485,3201,14171,225,1487,1321,12437,711,789,15,2613,1897,1953,709,5885,2515,4953,421,3905,7185,267,1311,341,3709,3417,1375,3731,729,3713,3505,5063,3645,3057,2409,6701,3265,93,111,3267,2469,1767,3451,4443,657,4149,1723,143,5017,1925,12369,1113,387,1965,1765,131,4249,669,1851,971,4575,1965,1521,1383,411,555,6093,5141,6051,3105,2751,11,727,2057,1003,3563,1309,1679,2623,5547,241,2253,2725,1511,4647,1107,6723,3881,415,923,1645,2523,591,1449,1101,6453,1657,3053,3649,1197,3489,2375,25,677,7129,767,13485,3815,571,2255,319,2555,1491,1007,669,1743,1185,2385,153,5297,799,1445,1111,2931,2247,1745,1083,2913,159,12779,451,3,6051,6135,549,4877,289,1137,5409,453,7009,987,1185,243,1681,3723,5239,3165,2625,1889,2499,2207,1377,1869,49,25253,3819,933,11749,245,777,4755,1189,5637,3451,165,6873,335,721,1245,5479,5675,1857,587,3981,5081,75,2303,129,797,4729,5955,2635,3267,2619,195,705,255,1081,8055,1395,2855,3007,2129,411,4431,3381,1605,4623,681,6237,1383,3463,2865,391,4815,1479,6383,3537,2375,739,2495,1141,507,861,507,87,2717,1489,1025,159,1403,1243,5253,5911,2157,2079,1157,15667,813,9315,407,1017,2067,349,27,1615,825,1275,17,445,7215,2071,1251,885,2505,3391,825,1137,6929,273,1031,5001,18297,1473,1281,507,2645,909,2403,1179,125,919,1875,2089,975,2329,503,2569,3857,7423,4571,1615,2549,783,567,645,3747,573,501,3369,6107,3015,183,2859,309,1935,2271,309,6309,2061,4487,2467,1997,1281,7983,1125,1547,685,3311,291,1203,1621,5955,4945,4563,1963,1535,447,377,3255,653,2479,705,739,1287,8499,927,5725,2385,14929,1253,4623,605,801,2633,7899,6551,3019,1139,2401,1377,5775,4025,873,3531,3535,1017,1665,4467,325,257,1095,483,1281,209,621,305,11511,3015,289,11373,4621,5285,991,603,4639,1917,6621,6017,729,3267,4185,147,1135,7095,5331,7125,481,363,909,10311,4075,4943,553,4857,547,5469,313,1373,1195,3755,381,741,2517,12039,165,321,2877,6093,6105,1461,16957,1529,379,1277,2251,1587,331,1415,3937,2103,2449,1875,2247,3275,861,2253,2737,2879,13,2045,1287,2139,439,131,7011,1197,2469,13943,1461,1559,3009,7155,1791,2217,253,227,451,6423,1,3507,2649,1325,1845,425,1885,5087,8949,575,1545,3363,2323,941,4047,3473,5509,843,2341,1307,1929,1931,7891,489,2215,1805,997,3765,1129,827,4831,2883,1711,4053,7279,4593,3349,1997,267,1853,2635,4905,37,3513,1455,3771,2079,9135,7345,5583,5397,387,2179,347,1575,1607,5599,3765,2779,5393,633,1887,171,6497,5091,935,10747,3965,3781,1745,6837,1265,3961,5877,1737,2987,4563,8103,301,4319,2785,16211,1629,1635,3655,627,1357,833,75,633,2367,1595,3769,2835,3205,8789,12463,501,195,9663,3345,273,385,4823,1495,2031,91,1295,7605,2813,2719,7103,699,11033,297,1139,4495,777,1071,3687,5955,483,6805,2933,2895,173,4135,345,6955,6533,4059,9995,535,4157,4087,3173,63,957,741,89,8721,15,817,5327,3645,327,2265,549,9721,3435,421,5187,2479,1757,1549,1799,3565,2343,3435,5147,885,2421,1315,2589,7551,12621,2389,459,3843,10163,1,1883,91,2043,1485,185,411,911,2799,3573,2641,5105,409,4557,5301,1715,141,3603,169,285,3727,10655,3813,1817,1557,2463,439,203,489,347,1741,377,4231,5153,321,95,8317,429,613,1007,4677,2067,1999,5963,2781,2559,3409,4821,2107,8465,865,7635,7087,3489,339,11711,6957,3449,2355,6327,13107,1965,391,477,367,147,3303,3731,3889,5985,1419,2357,6111,3335,2499,18927,1569,3077,669,201,3615,2247,8085,321,9745,9285,1255,2043,325,2447,3729,1703,75,45,2805,6027,4551,8675,3475,2381,1777,4505,1149,3521,2365,45,3951,1971,327,5,103,15533,129,6285,571,981,3469,4259,1099,3635,4339,363,5989,1775,1599,143,9859,1461,19,3713,405,1343,1315,9339,2601,1445,2329,11919,825,2051,997,519,111,1145,1567,197,1831,7125,667,537,15369,1367,3747,4667,1561,4287,2425,533,6625,671,6141,1319,4543,1883,135,1505,5631,2757,1897,315,4723,455,7735,689,1693,495,6241,2847,6823,9275,1639,89,3393,9317,849,5975,4189,525,615,3639,1965,2061,171,1343,5095,12561,4251,5355,1291,4577,1747,1863,6121,315,147,819,271,1281,2245,3509,3721,2277,157,749,2241,2097,2451,4229,4015,503,351,1043,1929,93,649,1379,8821,1785,5115,1193,625,7605,2215,345,181,3135,3129,4325,1755,2895,2467,10373,1809,2615,1015,717,4863,1641,1939,5279,1701,3477,3109,413,1299,833,49,6437,361,4083,1287,207,1821,5547,2107,5537,1105,3087,75,1763,5769,8325,4419,2453,2403,455,319,3465,829,297,187,993,8481,1011,2797,5103,1203,1691,237,825,11541,3903,1189,483,2385,561,51,6383,8121,185,3291,10377,1189,927,1447,147,343,6845,811,7983,849,1895,2245,2283,9433,4187,247,1265,2041,317,1351,8435,2259,11415,1065,3585,2935,2031,3625,8439,885,5531,525,3849,553,5117,3907,1983,459,17141,1087,5025,4935,357,1147,4257,2695,13305,411,2135,7011,2687,49,4575,1803,681,597,10175,1123,635,6579,3927,4231,27,5787,923,1099,1307,615,1767,1281,1203,2341,3489,3399,2447,1807,1727,45,9323,2019,2225,2401,1371,4857,10395,2103,717,691,2973,2881,1253,205,1865,1519,4671,1075,1689,1419,3647,4657,7865,1291,3773,481,1943,651,917,1111,1619,5109,1971,211,5577,679,7275,7245,8903,483,2141,1587,279,2713,8657,255,189,1995,5991,765,1985,351,1055,507,1493,2971,923,339,905,5005,4265,1575,6255,8965,2655,559,2205,819,2565,3979,3177,1585,5297,4459,2573,1323,257,1695,3713,9411,6063,1639,2295,3895,5015,9315,4599,4125,11741,265,3299,381,12143,3367,2855,61,111,5535,677,1233,833,8989,1137,691,393,4867,10589,5079,1077,12387,75,421,387,10267,5687,1785,1473,5005,423,399,273,6705,2009,735,2837,11911,5669,4771,10785,4987,569,6585,9717,355,2285,14511,3035,2007,5693,685,467,10501,123,1681,3015,2637,677,673,1955,5559,1139,15175,3441,2889,4325,7185,2157,13081,1023,3981,747,2629,485,415,2001,769,1953,6555,18801,7111,1785,1101,14033,10675,909,5383,6843,6099,719,1033,267,2469,5993,705,3413,6627,11055,3901,1457,14505,4853,5373,1443,3841,2423,2661,5373,1405,2159];

decrements.length = max_exponent - first_decrement;

var exec = function(setup, equ, is_arr)
{
	if (is_arr)
	{
		equ = '",".join([str(i) for i in ' + equ + '])';
	}

	var out = child_process.spawnSync('python', ['-c', setup + 'print(' + equ + ')']).stdout.toString();
	var res = out.trim();

	process.stderr.write(equ + ' -> ' + res + '\n');

	return is_arr ? res.split(',') : res;
};

var calc = function(equ, mod)
{
	var setup = '';
	setup += 'def wrap(n):\n\tif n < 0:\n\t\tn += (n // ' + mod + ' + 1) * ' + mod + '\n\treturn n % ' + mod + '\n';
	setup += 'def inv(n):\n\treturn pow(n, ' + mod + ' - 2, ' + mod + ')\n';

	return exec(setup, 'wrap(' + equ + ')');
};

var output = [];

output.push('// Generated file, do not modify. Make changes to tests/generate_intmodulomersenne.js');
output.push('');
output.push('#include "catch/single_include/catch.hpp"');
output.push('#include "libSonicSocket/intmodulomersenne.h"');
output.push('');

for (var i = 0; i < num_fields; i++)
{
	var exponent = Math.floor(Math.random() * decrements.length) + first_decrement;
	var decrement = decrements[exponent - first_decrement];
	var type = 'sonic_socket::IntModuloMersenne<' + exponent + ', ' + decrement + '>';

	var mod = exec('', '2 ** ' + exponent + ' - ' + decrement);
	var [a,b,c,d,e,f,g] = exec('import random\n', '[random.randrange(' + mod + ') for x in range(0, 7)]', true);

	if (a === '0' || b === '0' || g === '0')
	{
		// Can't div by zero, so try again
		i--;
		continue;
	}

	output.push('TEST_CASE("' + a + ' _ ' + b + ' (mod 2^' + exponent + ' - ' + decrement + ' == ' + mod + ')", "")');
	output.push('{');
	output.push('    typedef ' + type + ' Field;');
	output.push('');
	output.push('    Field a("' + a + '");');
	output.push('    Field b("' + b + '");');
	output.push('');
	output.push('    Field copy(a);');
	output.push('    REQUIRE(copy == a);');
	output.push('');
	output.push('    Field set;');
	output.push('    set = b;');
	output.push('    REQUIRE(set == b);');
	output.push('');
	output.push('    REQUIRE(a + b == Field("' + calc(a + ' + ' + b, mod) + '"));');
	output.push('    REQUIRE(a - b == Field("' + calc(a + ' - ' + b, mod) + '"));');
	output.push('    REQUIRE(b - a == Field("' + calc(b + ' - ' + a, mod) + '"));');
	output.push('    REQUIRE(a * b == Field("' + calc(a + ' * ' + b, mod) + '"));');
	output.push('    REQUIRE(a / b == Field("' + calc(a + ' * inv(' + b + ')', mod) + '"));');
	output.push('');
	output.push('    Field mut("' + c + '");');
	output.push('    mut += Field("' + d + '");');
	output.push('    REQUIRE(mut == Field("' + calc(c + ' + ' + d, mod) + '"));');
	output.push('    mut -= Field("' + e + '");');
	output.push('    REQUIRE(mut == Field("' + calc(c + ' + ' + d + ' - ' + e, mod) + '"));');
	output.push('    mut *= Field("' + f + '");');
	output.push('    REQUIRE(mut == Field("' + calc('(' + c + ' + ' + d + ' - ' + e + ') * ' + f, mod) + '"));');
	output.push('    mut /= Field("' + g + '");');
	output.push('    REQUIRE(mut == Field("' + calc('(' + c + ' + ' + d + ' - ' + e + ') * ' + f + ' * inv(' + g + ')', mod) + '"));');
	output.push('');
	output.push('    Field *inv = a.inverse();');
	output.push('    REQUIRE(*inv == Field("' + calc('inv(' + a + ')', mod) + '"));');
	output.push('');
	output.push('    REQUIRE(a.to_string() == "' + a + '");');
	output.push('');
	output.push('    REQUIRE(a == a);');
	output.push('    REQUIRE_FALSE(a == b);');
	output.push('');
	output.push('    REQUIRE(a != b);');
	output.push('    REQUIRE_FALSE(a != a);');
	output.push('');

	while (a.length < b.length) {a = '0' + a;}
	while (b.length < a.length) {b = '0' + b;}

	if (a > b)
	{
		var tmp = a;
		a = b;
		b = tmp;
	}

	output.push('    Field min("' + a + '");');
	output.push('    Field max("' + b + '");');
	output.push('');
	output.push('    REQUIRE(min < max);');
	output.push('    REQUIRE_FALSE(min < min);');
	output.push('    REQUIRE_FALSE(max < min);');
	output.push('');
	output.push('    REQUIRE(min <= max);');
	output.push('    REQUIRE(min <= min);');
	output.push('    REQUIRE_FALSE(max <= min);');
	output.push('');
	output.push('    REQUIRE(max > min);');
	output.push('    REQUIRE_FALSE(min > min);');
	output.push('    REQUIRE_FALSE(min > max);');
	output.push('');
	output.push('    REQUIRE(max >= min);');
	output.push('    REQUIRE(min >= min);');
	output.push('    REQUIRE_FALSE(min >= max);');
	output.push('}');
	output.push('');
}

process.stdout.write(output.join('\n'));