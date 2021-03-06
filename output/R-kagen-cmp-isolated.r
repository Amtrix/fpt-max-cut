#!/usr/bin/env Rscript

options("width"=230)
library("optparse")
suppressWarnings(suppressMessages(library("dplyr")))

option_list = list(
    make_option(c("-t", "--type"), type="character", default=NULL, 
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

if (is.null(opt$type) || is.null(opt$out)){
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
res_folder=""
col_vec = c("black","dodgerblue2","red2","green3", "purple", "green", "violet")
pnt_vec  = c(17,16,15,5,0)
pnt_vec1 = c(pnt_vec[[1]], pnt_vec[[1]], pnt_vec[[1]], pnt_vec[[1]], pnt_vec[[1]])
pnt_vec2 = c(pnt_vec[[2]], pnt_vec[[2]], pnt_vec[[2]], pnt_vec[[2]], pnt_vec[[2]])
columns  <- c('#sec','#it','#|V(G)|','#|E(G)|','#|V(Gk)|','#|E(Gk)|','#|Erem|','#CUTDIFF','#MQLIB(G)','#MQLIB(Gk)+CUT','#MQLIB.DIFF','#MQLIB.DIFF.SD','#LOCSOLVER(G)','#LOCSOLVER(Gk)+CUT',
              '#LOCSOLVER.DIFF','#LOCSOLVER.DIFF.SD','#LOCSEARCH(G)','#LOCSEARCH(Gk)+CUT','#LOCSEARCH.DIFF','#LOCSEARCH.DIFF.SD','#BIQMAC_T(G)', '#BIQMAC_T(Gk)',
              '#EE(G)','#EE(Gk)','#MAXCUT.BEST','#ABOVE_EE_PARAM_LOWB', '#ktime', '#file', 'kagentype')
#columnsB  <- c('#sec','#it','#|V(G)|','#|E(G)|','#|V(Gk)|','#|E(Gk)|','#|Erem|','#CUTDIFF','#MQLIB(G)','#MQLIB(G)+CUT','#MQLIB.DIFF','#MQLIB.DIFF.SD','#LOCSOLVER(G)','#LOCSOLVER(G)+CUT','#LOCSOLVER.DIFF','#LOCSOLVER.DIFF.SD','#locsearch(G)','#locsearch(Gk)+CUT','#locsearch.DIFF','#locsearch.DIFF.SD','#EE(G)','#EE(Gk)','#MAXCUT.BEST','#ABOVE_EE_PARAM_LOWB', '#ktime', '#file')



#Need readjustment for each case:
x_start_legend <- 3
nam_vec = c(
    "Removed: Reduction Rule 5+",
    "Removed: Reduction Rule 23",
    "Removed: Reduction Rule 25",
    "Removed: Reduction Rule 24")
    #"Removed: Reduction Rule 23")#, "original", "task a", "task b", "task c")
# r8, s2, s3, s5, s6
file=paste("./experiments/kernelization/n2048/isolated-aggregate/isolated-rules-", opt$type, sep="")
                                                            
# Read the results from the csv files
data_table      <- read.table(paste(res_folder, file, sep=""), comment.char = "#", col.names = columns)


#data_table$ratio_e     = 1 - (data_table[,"X..E.Gk.."]/data_table[,"X..E.G.."])
#data_table_cmp$ratio_e = 1 - (data_table_cmp[,"X..E.Gk.."]/data_table_cmp[,"X..E.G.."])

table_list = list(data_table)
res <- lapply(table_list, function(dtable) {
    dtable$ratio_e = 1 - (dtable[,"X..E.Gk.."]/dtable[,"X..E.G.."])
    dtable$ratio_v = 1 - (dtable[,"X..V.Gk.."]/dtable[,"X..V.G.."])
    dtable$density = dtable[,"X..E.G.."]/dtable[,"X..V.G.."]
   # dtable <- aggregate(. ~ X.sec+X.file, dtable, function(x) c(mean = min(x), sd = sd(x)))
   # dtable <- do.call("data.frame", dtable) # flatten

    # Set appropriate kagen type.
   # row_cnt <- nrow(dtable)
   # dtable$gtype = floor(dtable[,"X.sec"]/(row_cnt/5))
  #  dtable <- dtable[with(dtable, order(gtype, density.mean)), ]
    dtable
})

data_table <- res[[1]]

data_table_list <- list()
data_table_main <- dplyr::filter(data_table, X.sec == 0)
for (i in 1:4) {
    data_table_list[[i]] <- dplyr::filter(data_table, X.sec == i)
    data_table_list[[i]]$diff_v = data_table_list[[i]]$ratio_v - data_table_main$ratio_v
    data_table_list[[i]]$diff_e = data_table_list[[i]]$ratio_e - data_table_main$ratio_e

    print(paste(i,nrow(data_table_list[[i]])))
}

#print(data_table_list[[1]])

# Open a PDF to store the plot into
pdf(opt$out, width=10, height=5)

#data_table$diff_v = data_table$ratio_v.mean - data_table_cmp$ratio_v.mean
#data_table$diff_e = data_table$ratio_e.mean - data_table_cmp$ratio_e.mean

{
    par(cex = 1.1)
    par(mgp=c(4,1,0))
    par(mar=c(5,4.15,1,2)+0.1)

    # Here we choose the two comlumns, that we use for the plot
    y="diff_v"
    x="density"

    # Define some ranges for our plotting area
    xrange <- c(0,0)
    yrange <- c(0,0)
    for (i in 1:4) {
        xrange <- range(xrange, data_table_list[[i]][,x])
        yrange <- range(yrange, data_table_list[[i]][,y])


        #print(data_table_list[[i]][,y])
    }

   # print("Y:")
   # print(yrange)

    yrange <- yrange * 1.1
    yrange[2] <- max(-yrange[1], yrange[2])
    yrange[2] <- 0.04
    xrange[1] <- 0
    xrange[2] <- 7.9

    # Initialize the plotting area
    plot(xrange, yrange, yaxt='n',xaxt='n', xaxs='i', yaxs='i', col="black", type="n", main="stuff", ann=FALSE)
    axis(2, at=pretty(yrange), lab=paste0(pretty(yrange) * 100, '%'), las=TRUE)

    labs <-c("0","1","2","3","4","5","6","7","8","9","10")
    n=length(labs)
    axis(side=1, at=seq(1,n,2)-1, labels=labs[seq(1,n,2)])
    axis(side=1, at=seq(2,n,2)-1, labels=labs[seq(2,n,2)])


    # Label titles for both axes
    title(xlab=expression("Graph Density: " ~ frac(group("|",E(G),"|"),group("|",V(G),"|")), line=2.3))
    title(ylab=expression("e"[absDiff]*" = e(G"[new]*") - e(G"[old]*")"), line=3.2)
  #  title(main=expression("Absolute difference in efficiency: e"[absDiff]*" = e(G"[new]*") - e(G"[old]*")"))

    # Draws the 4 lines of measurements
    for (dx in 0:3) {
       sub <- data_table_list[[dx + 1]]
       #points(sub[,x] , sub[,y] , col=col_vec[[dx + 1]], pch=pnt_vec[[1]])

       cxval <- 1.3 - 0.25 * dx
       print(nam_vec[[dx + 1]])
       print(cxval)

       if (!opt$nopoints)
            points(sub[,x] , sub[,y] , col=col_vec[[dx + 1]], pch=pnt_vec[[dx + 1]], cex=cxval)

        if (do_loess) {
            loessv = loess_val
            if (dx == 1) loessv = 0.1

            lo <- loess(sub[[y]] ~ sub[[x]], sub, span=loessv)
            lines(sub[[x]], predict(lo), col=col_vec[[dx + 1]], lwd=2)
        }
    }

    lines(c(0,10), c(0,0), col=col_vec[[4]],lty=3)

    # Shows the legend
    legend("bottomright", yrange[2], nam_vec, lty=, col=col_vec, pch=pnt_vec)
}



# Closes the PDF
dev.off()