# Arnon Lutsky 213561640

Split_PGN(){
    input_file=$1
    dest_dir=$2
    game_num=0
    pgn_text=""
    while IFS= read -r line 
    do
        if [[ "$line" =~ ^\[Event\ \" ]]
        then
            if [[ -n "$pgn_text" ]]
            then
                game_num=$((game_num + 1))
                game_file="$dest_dir/capmemel24_${game_num}.pgn"
                echo "$pgn_text" > "$game_file"
                echo "Saved game to $game_file"
                pgn_text=""
            fi
        fi
        pgn_text+="$line"$'\n'
    done < "$input_file"

    if [[ -n "$pgn_text" ]]
    then
        game_num=$((game_num + 1))
        game_file="$dest_dir/capmemel24_${game_num}.pgn"
        echo "$pgn_text" > "$game_file"
        echo "Saved game to $game_file"
    fi

    echo "All games have been split and saved to '$dest_dir'."

}

if test "$#" -ne 2 
then
    echo "Usage: $0 <source_pgn_file> <destination_directory>"
    exit 1
fi

input_file=$1
dest_dir=$2

if test ! -f "$input_file"
then 
    echo "Error: File '$input_file' does not exist."
    exit 1
fi

if test ! -d "$dest_dir" 
then
    mkdir -p "$dest_dir"
    echo "Created directory '$dest_dir'."
fi

Split_PGN "$input_file" "$dest_dir"