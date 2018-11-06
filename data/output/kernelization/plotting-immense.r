#!/usr/bin/env Rscript
# THIS SCRIPT IS SPECIALLY USEFUL ALONGSIDE ${main-dir}/scripts/generate-output.sh (ALSO REASON IT WAS MADE :))

options("width"=230)
library("optparse")
library(dplyr)

option_list = list(
    make_option(c("-f", "--file"), type="character", default=NULL, 
              help="dataset file name", metavar="character"),
	make_option(c("-o", "--out"), type="character", default=NULL, 
              help="output file name [default= %default]", metavar="character")
); 
 
opt_parser = OptionParser(option_list=option_list);
opt = parse_args(opt_parser);

if (is.null(opt$file) || is.null(opt$out)){
  print_help(opt_parser)
  stop("At least one argument must be supplied (input file).n", call.=FALSE)
}


# Some variables that are used in various ways, to construct the plots
res_folder="./"
file=opt$file
col_vec = c("darkorange","red2","dodgerblue2","black", "purple")
pnt_vec = c(9,16,17,15,0)
columns  <- c('#sec','#it','#|V(G)|','#|E(G)|','#|V(Gk)|','#|E(Gk)|','#|Erem|','#CUTDIFF','#MQLIB(G)','#MQLIB(G)+DIFF','#MQLIBDIFF','#locsearch(G)','#locsearch(Gk)+DIFF','#locsearchDIFF','#locsearchDIFF.SD','#EE(G)','#EE(Gk)', '#ktime', '#file')


#Need readjustment for each case:
x_start <- 15
x_start_legend <- 9
case_type <- "ba_1024"
nam_vec = c("GNM","RGG2D","RGG3D","BA","RHG")#, "original", "task a", "task b", "task c")
                                                            
# Read the results from the csv files
data_table      <- read.table(paste(res_folder, file, sep=""), comment.char = "#", col.names = columns)

data_table$ratio_e = 1 - (data_table[,"X..E.Gk.."]/data_table[,"X..E.G.."])
data_table$density = data_table[,"X..E.G.."]/data_table[,"X..V.G.."]

data_table <- data_table[with(data_table, order(X.sec,density)), ]


# Open a PDF to store the plot into
pdf(opt$out, width=10, height=5)

print(data_table[,"X..V.G.."])
#print(paste("KEY:", getkey(0,data_table)))

v_count <- data_table[,"X..V.G.."][1]

for (entry in list(
    c("ratio_e", paste("Kernelization efficiency for KaGen graph instances", sep="")
    ))) {
    
    # Here we choose the two comlumns, that we use for the plot
    y=entry[1]
    x="density"

    # Define some ranges for our plotting area
    xrange <- range(x_start,data_table[,x])
    yrange <- range(0, data_table[,y])
    yrange[2] <- yrange[2] * 1.1
    print(yrange)
    # Initialize the plotting area
    plot(xrange, yrange, yaxs='i', col="black", type="n", main="stuff", ann=FALSE)

    # Label titles for both axes
    title(xlab="Graph density: |E| / |V|"     , line=2.3)
    title(ylab="kernelization efficiency:  1 - |E(G')| / |E(G)|", line=2.3)
    title(main=entry[2])

    # Draws the 4 lines of measurements
    for (dx in c(1,2,3,4,5)) {
       sub <- dplyr::filter(data_table, X.sec == dx - 1)
       points(sub[,x] , sub[,y] , col=col_vec[[dx]], pch=pnt_vec[[dx]])
    }
    # Shows the legend
    legend(x_start_legend, yrange[2], nam_vec, lty=, col=col_vec, pch=pnt_vec)
}



# Closes the PDF
dev.off()