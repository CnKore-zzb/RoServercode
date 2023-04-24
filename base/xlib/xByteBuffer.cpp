#include "xlib/xByteBuffer.h"
#include "xlib/xTools.h"
#include "xCommand.h"

xByteBuffer::xByteBuffer()
{
  resize(2 * MAX_BUFSIZE);
}

xByteBuffer::~xByteBuffer()
{
}

void xByteBuffer::resize(DWORD len)
{
  if (len <= _write_offset) return;

  _buffer.resize(len);
  _size = len;
}

void xByteBuffer::write(const void *data, DWORD len)
{
  wr_reserve(len);

  memcpy(getWriteBuf(), data, len);
  _write_offset += len;
}

void xByteBuffer::wr_reserve(DWORD len)
{
  if ((_write_offset + len) > size())
  {
    read_flip();
  }

  while ((_write_offset + len) > size())
  {
    resize(size() + MAX_BUFSIZE);
    XLOG_T("[xByteBuffer],%s,%s,缓冲区溢出,resize:%u", m_strName.c_str(), m_strType.c_str(), size());
  }
}

// 需使用前确认缓冲区大小 wr_reserve
DWORD xByteBuffer::write(DWORD len)
{
  if ((_write_offset + len) > size())
  {
    return 0;
  }

  _write_offset += len;

  return len;
}

void xByteBuffer::read(DWORD len)
{
  if (len <= getReadSize())
  {
    _read_offset += len;
  }
  else
  {
    _read_offset = _write_offset;
  }
}

void xByteBuffer::read_flip()
{
  if (_read_offset)
  {
    if (_write_offset > _read_offset)
    {
      DWORD offset = _write_offset - _read_offset;
      bcopy(&_buffer[_read_offset], &_buffer[0], offset);
      _read_offset = 0;
      _write_offset = offset;
    }
    else
    {
      _read_offset = _write_offset = 0;
    }
  }
}

void xByteBuffer::copy(xByteBuffer& buffer)
{
  if (size() < buffer.size())
  {
    resize(buffer.size());
  }
  if (buffer.wr_offset())
  {
    bcopy(buffer.getBegin(), this->getBegin(), buffer.wr_offset());
  }
  this->_write_offset = buffer.wr_offset();
  this->_read_offset = buffer.rd_offset();
}
