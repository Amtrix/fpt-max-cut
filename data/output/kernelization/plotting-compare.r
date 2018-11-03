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
types_to_test = c(0,1,2,3,4)
res_folder="./"
cmp_folder="./base/ver1/"
file=opt$file
col_vec = c("darkorange","red2","dodgerblue2","black", "purple")
pnt_vec  = c(20,4,18,15,0)
pnt_vec1 = c(pnt_vec[[1]], pnt_vec[[1]], pnt_vec[[1]], pnt_vec[[1]], pnt_vec[[1]])
pnt_vec2 = c(pnt_vec[[2]], pnt_vec[[2]], pnt_vec[[2]], pnt_vec[[2]], pnt_vec[[2]])
columns  <- c('#sec','#it','#|V(G)|','#|E(G)|','#|V(Gk)|','#|E(Gk)|','#|Erem|','#CUTDIFF','#MQLIB(G)','#MQLIB(G)+DIFF','#MQLIBDIFF','#locsearch(G)','#locsearch(Gk)+DIFF','#locsearchDIFF','#locsearchDIFF.SD','#EE(G)','#EE(Gk)', '#ktime', '#file')
 


#Need readjustment for each case:
x_start_legend <- 3
case_type <- "ba_1024"
nam_vec = c("GNM","RGG2D","RGG3D","BA","RHG")#, "original", "task a", "task b", "task c")
                                                            
# Read the results from the csv files
data_table      <- read.table(paste(res_folder, file, sep=""), comment.char = "#", col.names = columns)
data_table_cmp  <- read.table(paste(cmp_folder, file, sep=""), comment.char = "#", col.names = columns)

data_table$ratio_e     = 1 - (data_table[,"X..E.Gk.."]/data_table[,"X..E.G.."])
data_table_cmp$ratio_e = 1 - (data_table_cmp[,"X..E.Gk.."]/data_table_cmp[,"X..E.G.."])

data_table$density     = data_table[,"X..E.G.."]/data_table[,"X..V.G.."]
data_table_cmp$density = data_table_cmp[,"X..E.G.."]/data_table_cmp[,"X..V.G.."]
#data_table[,"X..E.G.."] <- log(data_table[,"X..E.G.."] , 2)

data_table <- data_table[with(data_table, order(X.sec,density)), ]
data_table_cmp <- data_table_cmp[with(data_table_cmp, order(X.sec,density)), ]


# Open a PDF to store the plot into
pdf(opt$out, width=10, height=5)

v_count <- data_table[,"X..V.G.."][1]
print(v_count)

# pnt_vec[[1]] == filled dots
# pnt_vec[[2]] == crosses

{
    # Here we choose the two comlumns, that we use for the plot
    y="ratio_e"
    x="density"

    # Define some ranges for our plotting area
    xrange <- range(data_table[,x])
    yrange <- range(0, data_table[,y])
    yrange[2] <- 1.1
    xrange[1] <- xrange[1] - 0.5
    xrange[2] <- xrange[2] + 1
    print(yrange)
    # Initialize the plotting area
    plot(xrange, yrange, yaxs='i', col="black", type="n", main="stuff", ann=FALSE)

    # Label titles for both axes
    title(xlab="Graph density: |E| / |V|"     , line=2.3)
    title(ylab="Kernelization efficiency:  e(G) = 1 - |E(G')| / |E(G)|", line=2.3)
    title(main="Comparative kernelization efficiency for KaGen graph instances")

    # Draws the 4 lines of measurements
    for (dx in types_to_test) {
       sub <- dplyr::filter(data_table, X.sec == dx)
       points(sub[,x] , sub[,y] , col=col_vec[[dx + 1]], pch=pnt_vec[[2]])
    }

    for (dx in types_to_test) {
       sub <- dplyr::filter(data_table_cmp, X.sec == dx)
       points(sub[,x] , sub[,y] , col=col_vec[[dx + 1]], pch=pnt_vec[[1]])
    }
    # Shows the legend
    legend(xrange[2] - 0.5, yrange[2], nam_vec, lty=, col=col_vec, pch=pnt_vec1)
}

data_table$diff_e = data_table$ratio_e - data_table_cmp$ratio_e

{
    # Here we choose the two comlumns, that we use for the plot
    y="diff_e"
    x="density"

    # Define some ranges for our plotting area
    xrange <- range(data_table[,x])
    yrange <- range(data_table[,y])
    yrange <- yrange * 1.1
    xrange[1] <- xrange[1] - 0.5
    xrange[2] <- xrange[2] + 1
    print(yrange)
    # Initialize the plotting area
    plot(xrange, yrange, yaxs='i', col="black", type="n", main="stuff", ann=FALSE)

    # Label titles for both axes
    title(xlab="Graph density: |E| / |V|"     , line=2.3)
    title(ylab=expression("Difference in efficiency:  e(G"[new]*") - e(G"[old]*")"), line=2.3)
    title(main="Change in efficiency")

    # Draws the 4 lines of measurements
    for (dx in types_to_test) {
       sub <- dplyr::filter(data_table, X.sec == dx)
       points(sub[,x] , sub[,y] , col=col_vec[[dx + 1]], pch=pnt_vec[[1]])
    }

    # Shows the legend
    legend(xrange[2] - 0.5, yrange[2], nam_vec, lty=, col=col_vec, pch=pnt_vec1)
}



# Closes the PDF
dev.off()