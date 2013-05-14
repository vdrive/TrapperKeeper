#pragma once
#include "BTorrent.h"

//Class will encapsulate the torrent and the process of transferring pieces, this is where we'll 
//store the bitmap and make all of the logic decisions on which piece to grab next.

class BTorrentProcess
{
public:
	BTorrentProcess(void);
	BTorrentProcess(BTorrent* torrent);
	~BTorrentProcess(void);


	struct piece_block
	{
		piece_block(int p_index, int b_index)
			: piece_index(p_index)
			, block_index(b_index)
		{}
		int piece_index;
		int block_index;
		bool operator==(const piece_block& b) const
		{ return piece_index == b.piece_index && block_index == b.block_index; }

		bool operator!=(const piece_block& b) const
		{ return piece_index != b.piece_index || block_index != b.block_index; }

	};


	struct block_info
		{
			block_info(): num_downloads(0) {}
			// the peer this block was requested or
			// downloaded from
			//address peer;
			// the number of times this block has been downloaded
			int num_downloads;
		};

		// this vector contains all pieces we don't have.
		// in the first entry (index 0) is a vector of all pieces
		// that no peer have, the vector at index 1 contains
		// all pieces that exactly one peer have, index 2 contains
		// all pieces exactly two peers have and so on.
		std::vector<std::vector<int> > m_piece_info;

		// this vector has the same structure as m_piece_info
		// but only contains pieces we are currently downloading
		// they have higher priority than pieces we aren't downloading
		// during piece picking
		std::vector<std::vector<int> > m_downloading_piece_info;

private:
	BTorrent * mp_torrent;
	int m_block_size;

	// Bitmap of torrent pieces
	vector<bool> m_piece_map;
	//number of pieces completed
	int m_pieces_completed;
	int m_map_size;
};
