# Arnon Lutsky 213561640
Game_Loop(){
    tot_moves=$1
    move_num=0
    Print_Board
    while true
    do
        echo "Move $move_num/$tot_moves"
        echo "Press 'd' to move forward, 'a' to move back, 'w' to go to the start, 's' to go to the end, 'q' to quit:"
        read key
        if [ "$key" = "q" ]
        then
            echo "Exiting."
            exit 0
        elif [ "$key" = "w" ]
        then
            move_num=0
            Reset_Board
            Print_Board
        elif [ "$key" = "d" ]
        then
            if [ $move_num -eq $tot_moves ]
            then
                echo "No more moves available."
            else
                Make_Move "$move_num"
                ((move_num++))
                Print_Board
            fi
        elif [ "$key" = "a" ]
        then
            if [ "$move_num" -eq 0 ]
            then
                Print_Board
            else
                ((move_num--))
                Play_To_Move "$move_num"
            fi
        elif [ "$key" = "s" ]
        then
            move_num=$tot_moves
            Play_To_Move "$move_num"
        else
            echo "Invalid key pressed: $key"
        fi
    done    
}
Print_Board(){
    echo "  a b c d e f g h  "
    for (( i=8; i>0; i-- )) 
    do
        echo -n "$i "
        for (( j=0; j<8; j++ ))
        do
            echo -n  ${board[(8-i)*8 + j]}" "
        done
        echo "$i"
    done
    echo "  a b c d e f g h  "
}
Make_Move(){
    curr_move=$1
    move=${uci_moves[$curr_move]}
    from_move=${move:0:2}
    to_move=${move:2:2}
    # from_tile=$(( (8 - ${from_move:1:1}) * 8 + $(($(printf "%d" "'${from_move:0:1}") - 97)) ))
    from_tile=$(( 8*( 8 - ${from_move:1:1}) + $(($(printf "%d" "'${from_move:0:1}") - 97)) ))
    to_tile=$(( 8 * (8 - ${to_move:1:1}) + $(($(printf "%d" "'${to_move:0:1}") - 97)) ))
    piece=${board[$from_tile]}
    lc_king_pos=$(( 8*( 8 - 8) + $(($(printf "%d" "'e") - 97)) ))
    uc_king_pos=$(( 8*( 8 - 1) + $(($(printf "%d" "'e") - 97)) ))
    # Promorion - change for the correct piece
    if [ -n "${move:4:1}" ]
    then
        piece=${move:4:1}
    fi
    # Edge cases - casteling, en pasant
    if [ "$move" = "e8g8" -a ${board[$lc_king_pos]} = "k" ]
    then
        board[$lc_king_pos]='.'
        board[$to_tile]="$piece"
        board[$(( $to_tile - 1))]="r"
        board[$(( $to_tile + 1))]="."
    elif [ "$move" = "e8c8" -a ${board[$lc_king_pos]} = "k" ]
    then
        board[$lc_king_pos]='.'
        board[$to_tile]="$piece"
        board[$(( $to_tile + 1))]="r"
        board[$(( $to_tile - 2))]="."
    elif [ "$move" = "e1g1" -a ${board[$uc_king_pos]} = "K" ]
    then
        board[$uc_king_pos]='.'
        board[$to_tile]="$piece"
        board[$(( $to_tile - 1))]="R"
        board[$(( $to_tile + 1))]="."
    elif [ "$move" = "e1c1" -a ${board[$uc_king_pos]} = "K" ]
    then
        board[$uc_king_pos]='.'
        board[$to_tile]="$piece"
        board[$(( $to_tile + 1))]="R"
        board[$(( $to_tile - 2))]="."
    elif [[ "${from_move:0:1}" != "${to_move:0:1}" && "${board[$to_tile]}" == "." && ( "${board[$from_tile]}" == "p" || "${board[$from_tile]}" == "P" ) ]]
        then
        if [ "$piece" == "p" ]
        then
            board[$from_tile]='.'
            board[$to_tile]="$piece"
            board[$(( $to_tile - 8))]='.'
        elif [ "$piece" == "P" ]
        then
            board[$from_tile]='.'
            board[$to_tile]="$piece"
            board[$(( $to_tile + 8))]='.'
        fi
    else
        board[$from_tile]='.'
        board[$to_tile]="$piece"

    fi
        
}
Play_To_Move(){
    curr_move_play_to_move=$1
    Reset_Board

    for (( k=0; k<$curr_move_play_to_move; k++ ))
    do
        Make_Move "$k"
    done

    Print_Board
}
Reset_Board(){
    board=(r n b q k b n r p p p p p p p p . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . P P P P P P P P R N B Q K B N R)
}
# pip install chess
input_file=$1
if test ! -f "$input_file"
then 
    echo "File does not exist: '$input_file'"
    exit 1
fi 

moves=""
getMoves=false
echo "Metadata from PGN file:"
while IFS= read -r line 
do
    if [[ "$line" =~ ^1\.\  ]] && ! $getMoves
    then
        moves+="$line "
        getMoves=true
    elif $getMoves
    then
        moves+="$line "
    else
        echo "$line"
    fi
    
done < "$input_file"
# echo "$moves"

uci_moves=($( python3 parse_moves.py "$moves" ))
tot_moves=${#uci_moves[@]}

board=(r n b q k b n r p p p p p p p p . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . P P P P P P P P R N B Q K B N R)

Game_Loop "$tot_moves"
