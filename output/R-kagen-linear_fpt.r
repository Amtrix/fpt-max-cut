#!/usr/bin/env Rscript
# THIS SCRIPT IS SPECIALLY USEFUL ALONGSIDE ${main-dir}/scripts/generate-output.sh (ALSO REASON IT WAS MADE :))

options("width"=230)
library("optparse")
library(dplyr)

option_list = list(
    make_option(c("-f", "--file"), type="character", default=NULL, 
              help="dataset file name", metavar="character"),

	make_option(c("-o", "--out"), type="character", default=NULL, 
              help="output file name [default= %default]", metavar="character"),

    make_option(c("--loess"), type="double", default=NULL, 
              help="loess factor, turned off if not provided", metavar="double"),


    make_option(c("--nopoints"), action="store_true", type="logical", default=FALSE, 
              help="If points should be omitted", metavar="logical")
); 
 
opt_parser = OptionParser(option_list=option_list);
opt = parse_args(opt_parser);

if (is.null(opt$file) || is.null(opt$out)){
    print_help(opt_parser)
    stop("At least one argument must be supplied (input file).n", call.=FALSE)
}

do_loess = TRUE
loess_val = 0.1;
if (is.null(opt$loess)) {
    do_loess = FALSE
} else {
    loess_val = opt$loess
}

# Some variables that are used in various ways, to construct the plots
res_folder=""
file=opt$file
col_vec = c("darkorange","red2","dodgerblue2","black", "purple")
pnt_vec = c(9,18,17,15,0)
columns  <- c('#sec','#it','#|V(G)|','#|E(G)|','#|V(Gk)|','#|E(Gk)|','#markedcnt','#markedtime','#twowaytime','#markedcnteduc','#markedcnteductime', '#markedcntrand','#file')


#Need readjustment for each case:
x_start <- 15
x_start_legend <- 9
case_type <- "ba_1024"
nam_vec = c("BA","GNM","RGG2D","RGG3D","RHG")#, "original", "task a", "task b", "task c")
                                                            
# Read the results from the csv files
data_table      <- read.table(paste(res_folder, file, sep=""), comment.char = "#", col.names = columns)


data_table$ratio_e = 1 - (data_table[,"X..V.Gk.."]/data_table[,"X..V.G.."])
data_table$density = data_table[,"X..E.G.."]/data_table[,"X..V.G.."]
data_table <- aggregate(. ~ X.sec+X.file, data_table, function(x) c(mean = min(x), sd = sd(x)))
data_table <- do.call("data.frame", data_table) # flatten

# Set appropriate kagen type.
row_cnt <- nrow(data_table)
data_table$gtype = floor(data_table[,"X.sec"]/(row_cnt/5))
data_table <- data_table[with(data_table, order(gtype, density.mean)), ]

# Open a PDF to store the plot into
pdf(opt$out, width=10, height=5)

#print(paste("KEY:", getkey(0,data_table)))

#v_count <- data_table[,"X..V.G.."][1]

#print(data_table)

{   
    par(cex = 1.2)
    # Here we choose the two comlumns, that we use for the plot
    y="ratio_e.mean"
    x="density.mean"

    # Define some ranges for our plotting area
    xrange <- range(data_table[,x])
    yrange <- range(0, data_table[,y])
    yrange[2] <- 1
    xrange[1] <- 0
    xrange[2] <- xrange[2] + 1

    # Initialize the plotting area
    plot(xrange, yrange, yaxt='n', xaxs='i', yaxs='i', col="black", type="n", main="stuff", ann=FALSE)
    axis(2, at=pretty(yrange), lab=paste0(pretty(yrange) * 100, '%'), las=TRUE)

    # Label titles for both axes
    title(xlab="Graph density: |E| / |V|"     , line=2.3)
    title(ylab="", line=2.3)
    title(main=bquote("Kernelization efficiency for KaGen graph instances; metric: e(G) = 1 - " ~ frac(group("|",V(G[ker]),"|"),group("|",V(G),"|"))))

    # Draws the 4 lines of measurements
    for (dx in c(1,2,3,4,5)) {
        sub <- dplyr::filter(data_table, gtype == dx - 1)
        sub <- do.call("data.frame", sub) # flatten
        
        if (!opt$nopoints)
            points(sub[,x] , sub[,y] , col=col_vec[[dx]], pch=pnt_vec[[dx]])

        if (do_loess) {
            lo <- loess(sub[[y]] ~ sub[[x]], sub, span=loess_val)
            lines(sub[[x]], predict(lo), col=col_vec[[dx]], lwd=4)
        }

        
    }
    # Shows the legend
    legend("topright", yrange[2], nam_vec, lty=, col=col_vec, pch=pnt_vec)
}




# Closes the PDF
dev.off()