#!/usr/bin/env Rscript

options("width"=230)
options(scipen = 999)
library("optparse")
library(dplyr)

printpoints <- FALSE
printloess <- TRUE
loessv <- 0.1
eliminate_in_kernelized_too <- TRUE


res_folder="./real-world/"
out_folder="./plots/"
col_vec = c("darkorange","red2","dodgerblue2","black",
            "purple", "palegreen4", "palevioletred4","khaki4","mediumorchid1", "aquamarine2")
pnt_vec  = c(18,4,18,15,0,18,4,18,15,0)
pnt_vec1 = c(pnt_vec[[1]], pnt_vec[[1]], pnt_vec[[1]], pnt_vec[[1]], pnt_vec[[1]], pnt_vec[[1]], pnt_vec[[1]], pnt_vec[[1]], pnt_vec[[1]], pnt_vec[[1]], pnt_vec[[1]], pnt_vec[[1]], pnt_vec[[1]], pnt_vec[[1]], pnt_vec[[1]])
columns <- c('sec','it','vg','eg','vgk','egk','erem','cutdiff','mqlibg','mqlibgkplus','mqlibdiff','mqlibdiffsd','localsolverg','localsolvergkplus','localsolverdiff','localsolverdiffsd','locsearchg','locsearchgk','locsearchdiff','locsearchdiffsd','eeg','eegk','maxcutsz','aboveee', 'ktime', 'file')

#c("sec", "numnodes","numedges","timex","maxcutsz","file")
#c('#sec','#it','#|V(G)|','#|E(G)|','#|V(Gk)|','#|E(Gk)|','#|Erem|','#CUTDIFF','#MQLIB(G)','#MQLIB(Gk)+CUT','#MQLIB.DIFF','#MQLIB.DIFF.SD','#LOCSOLVER(G)','#LOCSOLVER(Gk)+CUT','#LOCSOLVER.DIFF','#LOCSOLVER.DIFF.SD','#LOCSEARCH(G)','#LOCSEARCH(Gk)+CUT','#LOCSEARCH.DIFF','#LOCSEARCH.DIFF.SD','#EE(G)','#EE(Gk)','#MAXCUT.BEST','#ABOVE_EE_PARAM_LOWB', '#ktime', '#file')


data_table <- read.table(paste(res_folder, "out", sep=""), comment.char = "#", col.names = columns)

datasets <- unique(data_table[,"file"])

GetDatasetId <- function(name) {
    dx <- 0
    for (i in datasets) {
        if (i == name) {
            return(dx)
        }
        dx <- dx + 1
    }
    return(-2)
}

#data_table$xsec <- lapply(data_table[,"file"], function(x) GetDatasetId(x))

print(data_table)

data_table$avgdeg <- data_table$eg / data_table$vg
data_table$vrem <- 1 - (data_table$vgk / data_table$vg)
data_table$ktime <- data_table$ktime / 1000.0
data_table <- aggregate(. ~ file, data_table, function(x) c(mean = min(x), sd = sd(x)))

print(data_table)