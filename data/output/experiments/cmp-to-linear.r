#!/usr/bin/env Rscript

options("width"=230)
library("optparse")
library(dplyr)

option_list = list(
    make_option(c("-f", "--file"), type="character", default=NULL, 
              help="dataset file name", metavar="character"),

    make_option(c("-fl", "--filelinear"), type="character", default=NULL, 
              help="dataset file name of linear kernel", metavar="character"),

	make_option(c("-o", "--out"), type="character", default=NULL, 
              help="output file name [default= %default]", metavar="character"),

    make_option(c("--loess"), type="double", default=NULL, 
              help="loess factor, turned off if not provided", metavar="double"),


    make_option(c("--nopoints"), action="store_true", type="logical", default=FALSE, 
              help="If points should be omitted", metavar="logical")
); 

opt_parser = OptionParser(option_list=option_list);
opt = parse_args(opt_parser);

if (is.null(opt$file) || is.null(opt$out) || is.null(opt$filelinear)){
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
types_to_test = c(0,1,2,3,4)
res_folder="./"
col_vec = c("darkorange","red2","dodgerblue2","black", "purple")
pnt_vec  = c(18,4,18,15,0)
pnt_vec1 = c(pnt_vec[[1]], pnt_vec[[1]], pnt_vec[[1]], pnt_vec[[1]], pnt_vec[[1]])
pnt_vec2 = c(pnt_vec[[2]], pnt_vec[[2]], pnt_vec[[2]], pnt_vec[[2]], pnt_vec[[2]])
columnsA  <- c('#sec','#it','#|V(G)|','#|E(G)|','#|V(Gk)|','#|E(Gk)|','#|Erem|','#CUTDIFF','#MQLIB(G)','#MQLIB(G)+CUT','#MQLIB.DIFF','#MQLIB.DIFF.SD','#LOCSOLVER(G)','#LOCSOLVER(G)+CUT','#LOCSOLVER.DIFF','#LOCSOLVER.DIFF.SD','#locsearch(G)','#locsearch(Gk)+CUT','#locsearch.DIFF','#locsearch.DIFF.SD','#EE(G)','#EE(Gk)','#MAXCUT.BEST','#ABOVE_EE_PARAM_LOWB', '#ktime', '#file')
columnsB  <- c('#sec','#it','#|V(G)|','#|E(G)|','#|V(Gk)|','#|E(Gk)|','#markedcnt','#markedtime','#twowaytime','#markedcnteduc','#markedcnteductime', '#markedcntrand','#file')



#Need readjustment for each case:
x_start_legend <- 3
nam_vec = c("BA","GNM","RGG2D","RGG3D","RHG")#, "original", "task a", "task b", "task c")
                                                            
# Read the results from the csv files
data_table      <- read.table(paste(res_folder, opt$file, sep=""), comment.char = "#", col.names = columnsA)
data_table_cmp  <- read.table(paste(res_folder, opt$filelinear, sep=""), comment.char = "#", col.names = columnsB)

data_table$ratio_e     = 1 - (data_table[,"X..E.Gk.."]/data_table[,"X..E.G.."])
data_table_cmp$ratio_e = 1 - (data_table_cmp[,"X..E.Gk.."]/data_table_cmp[,"X..E.G.."])

table_list = list(data_table, data_table_cmp)
res <- lapply(table_list, function(dtable) {
    dtable$ratio_e = 1 - (dtable[,"X..E.Gk.."]/dtable[,"X..E.G.."])
    dtable$density = dtable[,"X..E.G.."]/dtable[,"X..V.G.."]
    dtable <- aggregate(. ~ X.sec+X.file, dtable, function(x) c(mean = min(x), sd = sd(x)))
    dtable <- do.call("data.frame", dtable) # flatten

    # Set appropriate kagen type.
    row_cnt <- nrow(dtable)
    dtable$gtype = floor(dtable[,"X.sec"]/(row_cnt/5))
    dtable <- dtable[with(dtable, order(gtype, density.mean)), ]
    dtable
})

data_table <- res[[1]]
data_table_cmp <- res[[2]]


# Open a PDF to store the plot into
pdf(opt$out, width=10, height=5)


data_table$diff_e = data_table$ratio_e.mean - data_table_cmp$ratio_e.mean

{
    # Here we choose the two comlumns, that we use for the plot
    y="diff_e"
    x="density.mean"

    # Define some ranges for our plotting area
    xrange <- range(data_table[,x])
    yrange <- range(data_table[,y])
    yrange <- yrange * 1.1
    yrange[2] <- 1
    xrange[1] <- 0
    xrange[2] <- xrange[2] + 1
    print(yrange)
    # Initialize the plotting area
    plot(xrange, yrange, yaxt='n', xaxs='i', yaxs='i', col="black", type="n", main="stuff", ann=FALSE)
    axis(2, at=pretty(yrange), lab=paste0(pretty(yrange) * 100, '%'), las=TRUE)

    # Label titles for both axes
    title(xlab="Graph density: |E| / |V|"     , line=2.3)
    title(ylab="", line=2.3)
    title(main=expression("Absolute difference in efficiency:  e(G"[new]*") - e(G"[old]*")"))

    # Draws the 4 lines of measurements
    for (dx in types_to_test) {
       sub <- dplyr::filter(data_table, gtype == dx)
       #points(sub[,x] , sub[,y] , col=col_vec[[dx + 1]], pch=pnt_vec[[1]])

       if (!opt$nopoints)
            points(sub[,x] , sub[,y] , col=col_vec[[dx + 1]], pch=pnt_vec[[1]])

        if (do_loess) {
            loessv = loess_val
            if (dx == 1) loessv = 0.1

            lo <- loess(sub[[y]] ~ sub[[x]], sub, span=loessv)
            lines(sub[[x]], predict(lo), col=col_vec[[dx + 1]], lwd=4)
        }
    }

    # Shows the legend
    legend("topright", yrange[2], nam_vec, lty=, col=col_vec, pch=pnt_vec1)
}



# Closes the PDF
dev.off()