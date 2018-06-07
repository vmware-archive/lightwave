#!/bin/bash

mdb_compact() {
    post_dir="/var/lib/vmware/post"
    comp_dir="/var/lib/vmware/post_com"
    bkp_dir="/var/lib/vmware/post_bkp"

    if [ ! -d "$post_dir" ]; then
        echo $post_dir" does not exist."
        return
    fi

    if [ -d "$post_dir/xlogs" ]; then
        if [ ! -z "$(ls $post_dir/xlogs)" ]; then
            echo "POST did not shutdown gracefully. Flushing WAL."
            /opt/vmware/bin/mdb_walflush
            if [ "$?" -ne 0 ]; then
                echo "Error while flushing WAL. Skipping compaction."
                return
            fi

            if [ ! -z "$(ls $post_dir/xlogs)" ]; then
                echo "WAL did not get flushed. Skipping compaction."
                return
            fi
        fi
    fi

    free_pages=$(mdb_stat -f $post_dir | grep "Free pages"| cut -f2 -d ":")
    total_pages=$(mdb_stat -e $post_dir | grep "Number of pages used"| cut -f2 -d ":")

    (( ratio=free_pages*100/total_pages ))
    echo "Free page percent is "$ratio

    if [ "$ratio" -ge 10 ]; then
        if [ -d "$comp_dir" ]; then
            rm -rf $comp_dir
        fi
        mkdir $comp_dir
        echo "Compacting DB."
        start_time="$(date -u +%s)"
        mdb_copy -c $post_dir $comp_dir
        if [ "$?" -ne 0 ]; then
            echo "Compacting failed. Continuing with original file."
            rm -rf $comp_dir
            return
        else
            end_time="$(date -u +%s)"
            elapsed=$(( end_time - start_time ))
            echo "Compacting successful in "$elapsed" seconds."
            s1=`du -h $post_dir/data.mdb | cut -f1`
            s2=`du -h $comp_dir/data.mdb | cut -f1`
            echo "Size before compacting = "$s1
            echo "Size after compacting  = "$s2
        fi
    else
        echo "Skipping compaction. Freepage percent is "$ratio
        return
    fi

    start_time="$(date -u +%s)"
    /opt/vmware/bin/mdb_verify_checksum -a $post_dir $comp_dir
    if [ "$?" -ne 0 ]; then
        end_time="$(date -u +%s)"
        elapsed=$(( end_time - start_time ))
        echo "Compacted DB does not match the original DB. Falling back to original DB."
        echo "Verification tool took "$elapsed" seconds."
        rm -rf $comp_dir
        return
    else
        end_time="$(date -u +%s)"
        elapsed=$(( end_time - start_time ))
        echo "Compacted DB matches original DB. Using compacted DB."
        echo "Verification tool took "$elapsed" seconds."
        if [ -d $bkp_dir ]; then
            rm -rf $bkp_dir
        fi
        mv $post_dir $bkp_dir
        mv $comp_dir $post_dir
    fi

    return
}
