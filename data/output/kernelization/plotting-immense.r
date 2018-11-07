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
columns  <- c('#sec','#it','#|V(G)|','#|E(G)|','#|V(Gk)|','#|E(Gk)|','#|Erem|','#CUTDIFF','#MQLIB(G)','#MQLIB(Gk)+CUT','#MQLIB.DIFF','#MQLIB.DIFF.SD','#LOCSOLVER(G)','#LOCSOLVER(Gk)+CUT','#LOCSOLVER.DIFF','#LOCSOLVER.DIFF.SD','#LOCSEARCH(G)','#LOCSEARCH(Gk)+CUT','#LOCSEARCH.DIFF','#LOCSEARCH.DIFF.SD','#EE(G)','#EE(Gk)','#MAXCUT.BEST','#ABOVE_EE_PARAM_LOWB', '#ktime', '#file')


#Need readjustment for each case:
x_start <- 15
x_start_legend <- 9
case_type <- "ba_1024"
nam_vec = c("BA","GNM","RGG2D","RGG3D","RHG")#, "original", "task a", "task b", "task c")
                                                            
# Read the results from the csv files
data_table      <- read.table(paste(res_folder, file, sep=""), comment.char = "#", col.names = columns)

data_table$ratio_e = 1 - (data_table[,"X..E.Gk.."]/data_table[,"X..E.G.."])
data_table$density = data_table[,"X..E.G.."]/data_table[,"X..V.G.."]

data_table <- data_table[with(data_table, order(X.sec,density)), ]


# Open a PDF to store the plot into
pdf(opt$out, width=10, height=5)

print(data_table)
#print(paste("KEY:", getkey(0,data_table)))

#v_count <- data_table[,"X..V.G.."][1]

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
    plot(xrange, yrange, yaxt='n', col="black", type="n", main="stuff", ann=FALSE)
    axis(2, at=pretty(yrange), lab=paste0(pretty(yrange) * 100, '%'), las=TRUE)

    # Label titles for both axes
    title(xlab="Graph density: |E| / |V|"     , line=2.3)
    title(ylab="", line=2.3)
    title(main=bquote("Kernelization efficiency for KaGen graph instances; metric: e(G) = 1 - " ~ frac(group("|",E(G[ker]),"|"),group("|",E(G),"|"))))

    # Draws the 4 lines of measurements
    for (dx in c(1,2,3,4,5)) {
       sub <- dplyr::filter(data_table, X.sec == dx - 1)
       points(sub[,x] , sub[,y] , col=col_vec[[dx]], pch=pnt_vec[[dx]])
    }
    # Shows the legend
    legend(xrange[2] - 0.75, yrange[2], nam_vec, lty=, col=col_vec, pch=pnt_vec)
}

#data_table$mqlib_chg = 1 - (data_table[,"X.MQLIB.G."]/data_table[,"X.MQLIB.Gk..CUT"])
#data_table$localsearch_chg = 1 - (data_table[,"X.locsearch.G."]/data_table[,"X.MQLIB.Gk..CUT"])
#data_table$localsolver_chg = 1 - (data_table[,"X.LOCSOLVER.G."]/data_table[,"X.LOCSOLVER.Gk..CUT"])

for (entry in list(c("X.MQLIB.DIFF","MQLIB"), c("X.LOCSEARCH.DIFF","LOCAL SEARCH"), c("X.LOCSOLVER.DIFF", "LOCALSOLVER")))
{   
    # Here we choose the two comlumns, that we use for the plot
    ysd=paste(entry[1], ".SD", sep="")
    y=entry[1]
    x="density"

    # Define some ranges for our plotting area
    xrange <- range(data_table[,x])
    yrange <- range(0, data_table[,y])
    yrange <- range(yrange, data_table[,y] + data_table[,ysd], data_table[,y] - data_table[,ysd])
    yrange <- yrange * 1.3

    xrange[1] <- xrange[1] - 0.5
    xrange[2] <- xrange[2] + 1
    print(yrange)
    # Initialize the plotting area
    plot(xrange, yrange, yaxt='n', col="black", type="n", main="stuff", ann=FALSE)
    axis(2, at=pretty(yrange), lab=paste0(pretty(yrange) * 100, '%'), las=TRUE)

    # Label titles for both axes
    title(xlab="Graph density: |E| / |V|"     , line=2.3)
    title(ylab="", line=2.7)
    title(main=paste("Maxcut improvement on result from ", entry[2], sep=""))

    # Draws the 4 lines of measurements
    for (dx in c(1,2,3,4,5)) {
       sub <- dplyr::filter(data_table, X.sec == dx - 1)
       points(sub[,x] , sub[,y] , col=col_vec[[dx]], pch=pnt_vec[[dx]])

       if (!all(sub[,ysd] == 0)) {
            arrows(sub[,x], sub[,y]-sub[,ysd], sub[,x], sub[,y]+sub[,ysd], length=0.05, angle=90, code=3, col=col_vec[[dx]])
        }
    }
    # Shows the legend
    legend(xrange[2] - 0.75, yrange[2], nam_vec, lty=, col=col_vec, pch=pnt_vec)
}



# Closes the PDF
dev.off()