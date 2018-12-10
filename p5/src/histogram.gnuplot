set title "DCT coefficients Histogram"
set ylabel 'Frequency'
set xlabel 'DCT coefficients'
set style histogram

set style fill solid 0.5 border lt -1

set boxwidth 0.75 relative

set xrange [-10:10]

plot 'histogram.dat' title 'Lena.tif' smooth freq w boxes

pause -1
