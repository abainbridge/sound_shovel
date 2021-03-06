// Own header
#include "sound_widget.h"

// Project headers
#include "app_gui.h"
#include "main.h"
#include "sound.h"
#include "sound_channel.h"
#include "sound_system.h"

#include "df_lib_plus_plus/binary_stream_readers.h"
#include "df_lib_plus_plus/binary_stream_writers.h"
#include "df_lib_plus_plus/clipboard.h"
#include "df_lib_plus_plus/gui/mouse_cursor.h"
#include "df_lib_plus_plus/gui/file_dialog.h"
#include "df_lib_plus_plus/gui/status_bar.h"

// Contrib headers
#include "df_bitmap.h"
#include "df_common.h"
#include "df_time.h"
#include "df_font.h"
#include "df_window.h"

// Standard headers
#include <limits.h>
#include <math.h>
#include <stdlib.h>


// ***************************************************************************
// Private Methods
// ***************************************************************************

static bool NearlyEqual(double a, double b)
{
    double diff = fabs(a - b);
    return diff < 1e-5;
}


void SoundWidget::AdvanceSelection()
{
    if (g_gui->m_focussedWidget == this && IsMouseInBounds())
    {
        if (g_input.lmbClicked)
        {
            m_selectionStart = GetSampleIndexFromScreenPos(g_input.mouseX);
            m_selectionEnd = -1;
            m_selecting = true;
        }
    }

    if (m_selecting) {
        if (g_input.lmb) {
            m_selectionEnd = GetSampleIndexFromScreenPos(g_input.mouseX);
            g_gui->m_canSleep = false;
        }
        else {
            m_selecting = false;
        }
    }
}


void SoundWidget::AdvancePlaybackPos()
{
    if (m_playbackIdx < 0)
        return;

    float pixelDistanceToTarget = fabs(m_playbackIdx - m_playbackPos) / m_hZoomRatio;
    if (NearlyEqual(pixelDistanceToTarget, 0.0))
        return;

    g_gui->m_canSleep = false;
    float advanceTime = g_window->advanceTime;
    if (advanceTime > 0.1)
    {
        m_playbackPos = m_playbackIdx;
    }
    else
    {
        for (int i = 0; i < 10; i++) 
        {
            float dampingFactor = g_window->advanceTime * 4.0;
            m_playbackPos = m_playbackIdx * dampingFactor + m_playbackPos * (1.0 - dampingFactor);
        }
    }
}


void SoundWidget::RenderMarker(DfBitmap *bmp, int64_t sampleIdx, DfColour col)
{
    double x = GetScreenPosFromSampleIndex(sampleIdx);
    double fractionalX = x - (int64_t)x;
    float const gammaCorrectionFactor = 0.75;
    DfColour c = col;
    c.a = col.a * powf(1.0 - fractionalX, gammaCorrectionFactor);
    VLine(bmp, floor(x), m_top, m_height, c);
    c.a = col.a * powf(fractionalX, gammaCorrectionFactor);
    VLine(bmp, ceil(x), m_top, m_height, c);
}


void SoundWidget::RenderWaveform(DfBitmap *bmp, double vZoomRatio)
{
    DfColour soundColour = Colour(52, 152, 219);
    int channelHeight = m_height / m_sound->m_numChannels;

    for (int chanIdx = 0; chanIdx < m_sound->m_numChannels; chanIdx++)
    {
        SoundChannel *chan = m_sound->m_channels[chanIdx];

        chan->CalcDisplayData(m_hOffset, m_displayMins, m_displayMaxes, m_width, m_hZoomRatio);

        int yMid = m_top + channelHeight * chanIdx + channelHeight / 2;

        for (unsigned x = 0; x < m_width; x++)
        {
            int vline_len = ceil((m_displayMaxes[x] - m_displayMins[x]) * vZoomRatio);
            int y = ceil(yMid - m_displayMaxes[x] * vZoomRatio);
            if (vline_len == 0)
                PutPix(bmp, m_left + x, y, soundColour);
            else
                VLine(bmp, m_left + x, y, vline_len, soundColour);
        }

        HLine(bmp, m_left, yMid, m_width, Colour(255, 255, 255, 60));
        HLine(bmp, m_left, yMid + 32767 * vZoomRatio, m_width, Colour(255, 255, 255, 60));
    }

#if 0
    // Render block boundaries
    SoundChannel *chan = m_sound->m_channels[0];
    int64_t sampleIdx = GetSampleIndexFromScreenPos(0);
    SoundChannel::SoundPos soundPos = chan->GetSoundPosFromSampleIdx(sampleIdx);
    sampleIdx -= soundPos.m_sampleIdx;
    while (1)
    {
        int x = GetScreenPosFromSampleIndex(sampleIdx);
        if (x > m_width)
            break;
        VLine(bmp, x, m_top, 100, g_colourWhite);
        sampleIdx += chan->m_blocks[soundPos.m_blockIdx]->m_len;
        soundPos.m_blockIdx++;
    }
#endif
}


void SoundWidget::RenderSelection(DfBitmap *bmp)
{
    DfColour col = Colour(255, 40, 59, 63);

    if (m_selectionEnd >= 0)
    {
        int64_t selectionStart = m_selectionStart;
        int64_t selectionEnd = m_selectionEnd;
        if (selectionEnd < selectionStart)
        {
            selectionStart = m_selectionEnd;
            selectionEnd = m_selectionStart;
        }

        double x1 = GetScreenPosFromSampleIndex(selectionStart);
        double x2 = GetScreenPosFromSampleIndex(selectionEnd);
        RectFill(bmp, x1, m_top, x2 - x1 + 1, m_height, col);
    }
    else
    {
        RenderMarker(bmp, m_selectionStart, col);
    }
}


// ***************************************************************************
// Public Methods
// ***************************************************************************

SoundWidget::SoundWidget(Widget *parent)
    : Widget(SOUND_VIEW_NAME, parent)
{
    m_sound = NULL;
    m_displayMins = NULL;
    m_displayMaxes = NULL;

    Close();
//     Open("c:/users/andy/desktop/andante.wav");
//     m_selectionStart = 3e6;
//     m_selectionEnd = 3.3e6;
    m_selecting = false;

    g_soundSystem->PlaySound(this);
}


bool SoundWidget::Open(char const *filename)
{
    Close();
    m_sound = new Sound();
    BinaryFileReader binFileReader(filename);
    return m_sound->LoadWav(&binFileReader);
}


bool SoundWidget::OpenDialog()
{
    DArray <String> filenames = FileDialogOpen("C:/users/andy/desktop");
    if (filenames.Size() == 0)
        return false;
    
    if (filenames.Size() > 1)
    {
        g_statusBar->ShowError("Too many files selected");
        return false;
    }
    
    bool ok = Open(filenames[0].c_str());
    if (ok)
        g_statusBar->ShowMessage("Opened %s", filenames[0]);
    else
        g_statusBar->ShowError("Couldn't open %s", filenames[0]);

    return ok;
}


void SoundWidget::Close()
{
    if (m_sound)
    {
        delete m_sound;
        m_sound = NULL;
    }

    m_hOffset = 0.0;
    m_targetHOffset = 0.0;
    m_hZoomRatio = m_targetHZoomRatio = -1.0;

    m_playbackPos = -1.0;

    m_selectionStart = -1.0;
    m_selectionEnd = -1.0;

    m_playbackIdx = 0;
    m_isPlaying = false;
}


void SoundWidget::Delete()
{
    int64_t startIdx, endIdx;
    GetSelectionBlock(&startIdx, &endIdx);
    m_sound->Delete(startIdx, endIdx);
    m_selectionEnd = -1;
}


void SoundWidget::Copy()
{
    int64_t selectionStart, selectionEnd;
    GetSelectionBlock(&selectionStart, &selectionEnd);
    if (selectionEnd > 0) {
        BinaryDataWriter dataWriter;
        m_sound->SaveWav(&dataWriter, selectionStart, selectionEnd);
        g_clipboard.SetData(Clipboard::TYPE_WAV, dataWriter.m_data, dataWriter.m_dataLen);
    }
}


void SoundWidget::Paste()
{
    void *wavData = g_clipboard.GetData(Clipboard::TYPE_WAV);
    if (!wavData)
        return;
    BinaryDataReader dataReader((unsigned char *)wavData, INT_MAX, "clipboard");
    Sound *s = new Sound;
    s->LoadWav(&dataReader);
    m_sound->Insert(m_selectionStart, s);   // Ownership of s transfers to Insert().
    g_clipboard.ReleaseData(wavData);
}


void SoundWidget::GetSelectionBlock(int64_t *startIdx, int64_t *endIdx)
{
    if (m_selectionStart < m_selectionEnd || m_selectionEnd < 0)
    {
        *startIdx = m_selectionStart;
        *endIdx = m_selectionEnd;
    }
    else
    {
        *startIdx = m_selectionEnd;
        *endIdx = m_selectionStart;
    }
}


void SoundWidget::TogglePlayback()
{
    if (!m_sound) return;
    m_isPlaying = !m_isPlaying;
}


void SoundWidget::Play()
{
    if (!m_sound) return;
    m_isPlaying = true;
}


void SoundWidget::Pause()
{
    if (!m_sound) return;
    m_isPlaying = false;
}


void SoundWidget::FadeIn()
{
    if (!m_sound) return;
    int64_t startIdx, endIdx;
    GetSelectionBlock(&startIdx, &endIdx);
    m_sound->FadeIn(startIdx, endIdx);
}


void SoundWidget::FadeOut()
{
    if (!m_sound) return;
    int64_t startIdx, endIdx;
    GetSelectionBlock(&startIdx, &endIdx);
    m_sound->FadeOut(startIdx, endIdx);
}


void SoundWidget::Normalize()
{
    if (!m_sound) return;
    int64_t startIdx, endIdx;
    GetSelectionBlock(&startIdx, &endIdx);
    m_sound->Normalize(startIdx, endIdx);
}


void SoundWidget::SetRect(int x /* = -1 */, int y /* = -1 */, int w /* = -1 */, int h /* = -1 */)
{  
    if (w != m_width)
    {
        delete[] m_displayMins;
        delete[] m_displayMaxes;

        m_displayMins = new int16_t[w];
        m_displayMaxes = new int16_t[w];
    }

    Widget::SetRect(x, y, w, h);
}


void SoundWidget::Advance()
{
    if (!m_sound) return;

    if (m_hZoomRatio < 0.0)
        return;

    AdvanceSelection();

    double hZoomRatioBefore = m_hZoomRatio;
    double maxHOffset = m_sound->GetLength() - m_width * m_hZoomRatio;
    maxHOffset = IntMax(0.0, maxHOffset);

    double advanceTime = g_window->advanceTime;

    // If we are asleep, advanceTime will be large. If the user gave input 
    // that will wake us in this frame then we will end up using the large
    // time step for the first frame of the physics advance. This makes zoom
    // and scroll actions appear to start with a jolt, which looks 
    // unpleasant. Simply setting a fake small advanceTime for frames that
    // start in sleep fixes this.
    if (g_gui->m_canSleep)
        advanceTime = 0.01;


    //
    // Take mouse input

    double const ZOOM_INCREMENT = 1.3;
    if (g_input.mouseVelZ < 0) {
        for (int i = 0; i < -g_input.mouseVelZ; i++) {
            m_targetHZoomRatio *= ZOOM_INCREMENT;
        }
    }
    else if (g_input.mouseVelZ > 0) {
        for (int i = 0; i < g_input.mouseVelZ; i++) {
            m_targetHZoomRatio /= ZOOM_INCREMENT;
        }
    }

    if (g_input.mmb)
    {
        g_gui->m_mouseCursor.RequestCursorType(MouseCursor::CursorDragBoth);
        m_hOffset -= g_input.mouseVelX * m_hZoomRatio;
        m_targetHOffset = m_hOffset - g_input.mouseVelX * m_hZoomRatio;
        g_gui->m_canSleep = false;
    }
    else if (g_input.mmbUnClicked)
    {
        m_targetHOffset -= g_input.mouseVelX * m_hZoomRatio * 50.0;
    }

    if (g_input.lmbClicked && IsMouseInBounds())
    { 
        m_playbackIdx = GetSampleIndexFromScreenPos(g_input.mouseX);
        m_playbackPos = m_playbackIdx;
    }


    //
    // Take keyboard input

    const double KEY_ZOOM_INCREMENT = 1.0 + 1.3 * advanceTime;
    if (g_input.keys[KEY_UP])
        m_targetHZoomRatio /= KEY_ZOOM_INCREMENT;
    if (g_input.keys[KEY_DOWN])
        m_targetHZoomRatio *= KEY_ZOOM_INCREMENT;
    if (g_input.keyDowns[KEY_PGUP])
        m_targetHZoomRatio /= KEY_ZOOM_INCREMENT * 8.0;
    if (g_input.keyDowns[KEY_PGDN])
        m_targetHZoomRatio *= KEY_ZOOM_INCREMENT * 8.0;

    double const KEY_H_SCROLL_IMPLUSE = 1000.0 * m_hZoomRatio * advanceTime;
    if (g_input.keys[KEY_LEFT])
        m_targetHOffset -= KEY_H_SCROLL_IMPLUSE;
    if (g_input.keys[KEY_RIGHT])
        m_targetHOffset += KEY_H_SCROLL_IMPLUSE;

    if (g_input.keyDowns[KEY_HOME])
    {
        m_targetHOffset = 0;
    }
    else if (g_input.keyDowns[KEY_END])
    {
        double samples_on_screen = m_width * m_hZoomRatio;
        m_targetHOffset = m_sound->GetLength() - samples_on_screen;
    }

    if (g_input.keyDowns[KEY_SPACE])
        m_isPlaying = !m_isPlaying;


    //
    // Enforce constraints

    double maxHZoomRatio = m_sound->GetLength() / (double)m_width;
    if (m_targetHZoomRatio > maxHZoomRatio)
        m_targetHZoomRatio = maxHZoomRatio;

    double minHZoomRatio = 1.0;
    if (m_targetHZoomRatio < minHZoomRatio)
        m_targetHZoomRatio = 1.0;


    //
    // Do physics

    // Zoom
    double zoomBlendFactor = advanceTime * 18.0;
    m_hZoomRatio = (1.0 - zoomBlendFactor) * m_hZoomRatio + zoomBlendFactor * m_targetHZoomRatio;

    // H offset
    {
        double factor1 = 10.0 * advanceTime;
        double factor2 = 1.0 - factor1;
        m_hOffset = factor2 * m_hOffset + factor1 * m_targetHOffset;
    }


    //
    // Correct h_offset to compensate for zoom change this frame.

    {
        // The aim is to keep some part of the visible waveform in the same
        // place on the display. If there is a selection start, we use that,
        // otherwise we just use the sample that is currently in the middle
        // of the display.
        double numSamplesVisible = m_width * hZoomRatioBefore;
        double keepThing = (m_selectionStart - m_hOffset) / numSamplesVisible;
        if (keepThing < 0.0 || keepThing > 1.0)
            keepThing = 0.5;

        double deltaHZoom = hZoomRatioBefore / m_hZoomRatio;

        if (deltaHZoom < 1.0)
        {
            double tmp = (deltaHZoom - 1.0) * numSamplesVisible;
            tmp *= keepThing / deltaHZoom;
            m_targetHOffset += tmp;
            m_hOffset += tmp;
        }
        else
        {
            double tmp = -((1.0 - deltaHZoom) * numSamplesVisible);
            tmp *= keepThing / deltaHZoom;
            m_targetHOffset += tmp;
            m_hOffset += tmp;
        }
    }


    //
    // Enforce constraints

    m_hOffset = ClampDouble(m_hOffset, 0.0, maxHOffset);
    m_targetHOffset = ClampDouble(m_targetHOffset, 0.0, maxHOffset);
    if (m_hZoomRatio < 1.0)
        m_hZoomRatio = 1.0;

    if (!NearlyEqual(m_hZoomRatio, m_targetHZoomRatio) ||
        !NearlyEqual(m_hOffset, m_targetHOffset))
    {
        g_gui->m_canSleep = false;
    }

    AdvancePlaybackPos();
}


void SoundWidget::Render()
{
    if (!m_sound) return;

    if (m_hZoomRatio < 0.0)
    {
        m_hZoomRatio = (double)m_sound->GetLength() / (double)m_width;
        m_targetHZoomRatio = m_hZoomRatio;
    }

    double vZoomRatio = (double)m_height / (65536 * m_sound->m_numChannels);

    RenderWaveform(g_window->bmp, vZoomRatio);
    RenderSelection(g_window->bmp);

    if (m_playbackIdx)
        RenderMarker(g_window->bmp, m_playbackPos, Colour(255, 255, 255, 90));
}


char *SoundWidget::ExecuteCommand(char const *object, char const *command, char const *arguments)
{
    if (0);
    else if (COMMAND_IS("Close"))       Close();
    else if (COMMAND_IS("Copy"))        Copy();
    else if (COMMAND_IS("Delete"))      Delete();
    else if (COMMAND_IS("FadeIn"))      FadeIn();
    else if (COMMAND_IS("FadeOut"))     FadeOut();
    else if (COMMAND_IS("Normalize"))   Normalize();
    else if (COMMAND_IS("OpenDialog"))  OpenDialog();
    else if (COMMAND_IS("Paste"))       Paste();
    else if (COMMAND_IS("Pause"))       Pause();
    else if (COMMAND_IS("Play"))        Play();
    else if (COMMAND_IS("Save"))        m_sound->SaveWav();
    else if (COMMAND_IS("TogglePlay"))  TogglePlayback();

    return NULL;
}


int64_t SoundWidget::GetSampleIndexFromScreenPos(int screenX)
{
    if (!m_sound) return 0;

    int64_t rv = m_hOffset + (double)(screenX - m_left) * m_hZoomRatio + 0.5;
    if (rv < 0)
        rv = 0;
    else if (rv >= m_sound->GetLength())
        rv = m_sound->GetLength() - 1;
    return rv;
}


double SoundWidget::GetScreenPosFromSampleIndex(int64_t sampleIdx)
{
    if (!m_sound) return 0.0;

    return m_left + ((double)sampleIdx - m_hOffset) / m_hZoomRatio;
}
