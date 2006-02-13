//GalaxySetupWnd.cpp
#include "GalaxySetupWnd.h"

#include "CUIControls.h"
#include "CUISpin.h"
#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"

#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>


namespace {
    const int CONTROL_MARGIN = 5;
    const int CONTROL_HEIGHT = 30;
    const int PANEL_CONTROL_SPACING = 33;
    const int GAL_SETUP_PANEL_HT = PANEL_CONTROL_SPACING * 6;
    const int GAL_SETUP_WND_WD = 645;
    const int GAL_SETUP_WND_HT = 326;
    const int RADIO_BN_HT = ClientUI::PTS + 4;
    const int RADIO_BN_SPACING = RADIO_BN_HT + 10;
    const GG::Pt PREVIEW_SZ(248, 186);
    const bool ALLOW_NO_STARLANES = false;

    bool temp_header_bool = RecordHeaderFile(GalaxySetupWndRevision());
    bool temp_source_bool = RecordSourceFile("$Id$");
}

////////////////////////////////////////////////
// GalaxySetupPanel
////////////////////////////////////////////////
GalaxySetupPanel::GalaxySetupPanel(int x, int y, int w/* = DEFAULT_WIDTH*/) :
    GG::Control(x, y, w, GAL_SETUP_PANEL_HT, 0),
    m_stars_spin(0),
    m_galaxy_shapes_list(0),
    m_galaxy_ages_list(0),
    m_starlane_freq_list(0),
    m_planet_density_list(0),
    m_specials_freq_list(0)
{
    TempUISoundDisabler sound_disabler;

    const int LABELS_WIDTH = (w - CONTROL_MARGIN) / 2;
    const int DROPLIST_WIDTH = LABELS_WIDTH;
    const int DROPLIST_HEIGHT = ClientUI::PTS + 4;
    const int TEXT_ROW_HEIGHT = CUISimpleDropDownListRow::DEFAULT_ROW_HEIGHT;
    const int MAX_DROPLIST_DROP_HEIGHT = TEXT_ROW_HEIGHT * 5;
    const int TOTAL_LISTBOX_MARGIN = 4;
    int row = -1;

    boost::shared_ptr<GG::Font> font = GG::GUI::GetGUI()->GetFont(ClientUI::FONT, ClientUI::PTS);

    AttachChild(new GG::TextControl(CONTROL_MARGIN, ++row * PANEL_CONTROL_SPACING, LABELS_WIDTH, CONTROL_HEIGHT, UserString("GSETUP_STARS"), font, ClientUI::TEXT_COLOR, GG::TF_RIGHT));
    m_stars_spin = new CUISpin<int>(LABELS_WIDTH + 2 * CONTROL_MARGIN, row * PANEL_CONTROL_SPACING, 75, 100, 1, 10, 500, true);
    m_stars_spin->OffsetMove(GG::Pt(0, (PANEL_CONTROL_SPACING - m_stars_spin->Height()) / 2));

    AttachChild(new GG::TextControl(CONTROL_MARGIN, ++row * PANEL_CONTROL_SPACING, LABELS_WIDTH, CONTROL_HEIGHT, UserString("GSETUP_SHAPE"), font, ClientUI::TEXT_COLOR, GG::TF_RIGHT));
    int drop_height = std::min(TEXT_ROW_HEIGHT * Universe::GALAXY_SHAPES, MAX_DROPLIST_DROP_HEIGHT) + TOTAL_LISTBOX_MARGIN;
    m_galaxy_shapes_list = new CUIDropDownList(LABELS_WIDTH + 2 * CONTROL_MARGIN, row * PANEL_CONTROL_SPACING, DROPLIST_WIDTH, DROPLIST_HEIGHT, drop_height);
    m_galaxy_shapes_list->OffsetMove(GG::Pt(0, (PANEL_CONTROL_SPACING - m_galaxy_shapes_list->Height()) / 2));
    m_galaxy_shapes_list->SetStyle(GG::LB_NOSORT);

    AttachChild(new GG::TextControl(CONTROL_MARGIN, ++row * PANEL_CONTROL_SPACING, LABELS_WIDTH, CONTROL_HEIGHT, UserString("GSETUP_AGE"), font, ClientUI::TEXT_COLOR, GG::TF_RIGHT));
    drop_height = std::min(TEXT_ROW_HEIGHT * Universe::NUM_UNIVERSE_AGES, MAX_DROPLIST_DROP_HEIGHT) + TOTAL_LISTBOX_MARGIN;
    m_galaxy_ages_list = new CUIDropDownList(LABELS_WIDTH + 2 * CONTROL_MARGIN, row * PANEL_CONTROL_SPACING, DROPLIST_WIDTH, DROPLIST_HEIGHT, drop_height);
    m_galaxy_ages_list->OffsetMove(GG::Pt(0, (PANEL_CONTROL_SPACING - m_galaxy_ages_list->Height()) / 2));
    m_galaxy_ages_list->SetStyle(GG::LB_NOSORT);

    AttachChild(new GG::TextControl(CONTROL_MARGIN, ++row * PANEL_CONTROL_SPACING, LABELS_WIDTH, CONTROL_HEIGHT, UserString("GSETUP_STARLANE_FREQ"), font, ClientUI::TEXT_COLOR, GG::TF_RIGHT));
    drop_height = std::min(TEXT_ROW_HEIGHT * Universe::NUM_STARLANE_FREQENCIES, MAX_DROPLIST_DROP_HEIGHT) + TOTAL_LISTBOX_MARGIN;
    m_starlane_freq_list = new CUIDropDownList(LABELS_WIDTH + 2 * CONTROL_MARGIN, row * PANEL_CONTROL_SPACING, DROPLIST_WIDTH, DROPLIST_HEIGHT, drop_height);
    m_starlane_freq_list->OffsetMove(GG::Pt(0, (PANEL_CONTROL_SPACING - m_starlane_freq_list->Height()) / 2));
    m_starlane_freq_list->SetStyle(GG::LB_NOSORT);

    AttachChild(new GG::TextControl(CONTROL_MARGIN, ++row * PANEL_CONTROL_SPACING, LABELS_WIDTH, CONTROL_HEIGHT, UserString("GSETUP_PLANET_DENSITY"), font, ClientUI::TEXT_COLOR, GG::TF_RIGHT));
    drop_height = std::min(TEXT_ROW_HEIGHT * Universe::NUM_UNIVERSE_PLANET_DENSITIES, MAX_DROPLIST_DROP_HEIGHT) + TOTAL_LISTBOX_MARGIN;
    m_planet_density_list = new CUIDropDownList(LABELS_WIDTH + 2 * CONTROL_MARGIN, row* PANEL_CONTROL_SPACING, DROPLIST_WIDTH, DROPLIST_HEIGHT, drop_height);
    m_planet_density_list->OffsetMove(GG::Pt(0, (PANEL_CONTROL_SPACING - m_planet_density_list->Height()) / 2));
    m_planet_density_list->SetStyle(GG::LB_NOSORT);

    AttachChild(new GG::TextControl(CONTROL_MARGIN, ++row* PANEL_CONTROL_SPACING, LABELS_WIDTH, CONTROL_HEIGHT, UserString("GSETUP_SPECIALS_FREQ"), font, ClientUI::TEXT_COLOR, GG::TF_RIGHT));
    drop_height = std::min(TEXT_ROW_HEIGHT * Universe::NUM_SPECIALS_FREQENCIES, MAX_DROPLIST_DROP_HEIGHT) + TOTAL_LISTBOX_MARGIN;
    m_specials_freq_list = new CUIDropDownList(LABELS_WIDTH + 2 * CONTROL_MARGIN, row* PANEL_CONTROL_SPACING, DROPLIST_WIDTH, DROPLIST_HEIGHT, drop_height);
    m_specials_freq_list->OffsetMove(GG::Pt(0, (PANEL_CONTROL_SPACING - m_specials_freq_list->Height()) / 2));
    m_specials_freq_list->SetStyle(GG::LB_NOSORT);

    Init();
}

int GalaxySetupPanel::Systems() const
{
    return m_stars_spin->Value();
}

Universe::Shape GalaxySetupPanel::GalaxyShape() const
{
    return Universe::Shape(m_galaxy_shapes_list->CurrentItemIndex());
}

Universe::Age GalaxySetupPanel::GalaxyAge() const
{
    return Universe::Age(m_galaxy_ages_list->CurrentItemIndex());
}

Universe::StarlaneFrequency GalaxySetupPanel::StarlaneFrequency() const
{
    return Universe::StarlaneFrequency(m_starlane_freq_list->CurrentItemIndex() + (ALLOW_NO_STARLANES ? 0 : 1));
}

Universe::PlanetDensity GalaxySetupPanel::PlanetDensity() const
{
    return Universe::PlanetDensity(m_planet_density_list->CurrentItemIndex());
}

Universe::SpecialsFrequency GalaxySetupPanel::SpecialsFrequency() const
{
    return Universe::SpecialsFrequency(m_specials_freq_list->CurrentItemIndex());
}

boost::shared_ptr<GG::Texture> GalaxySetupPanel::PreviewImage() const
{
    return m_textures[GalaxyShape()];
}

XMLElement GalaxySetupPanel::XMLEncode() const
{
    using boost::lexical_cast;
    using std::string;

    XMLElement retval("universe_params");
    retval.AppendChild(XMLElement("size", lexical_cast<string>(Systems())));
    retval.AppendChild(XMLElement("shape", lexical_cast<string>(GalaxyShape())));
    retval.AppendChild(XMLElement("age", lexical_cast<string>(GalaxyAge())));
    retval.AppendChild(XMLElement("starlane_freq", lexical_cast<string>(StarlaneFrequency())));
    retval.AppendChild(XMLElement("planet_density", lexical_cast<string>(PlanetDensity())));
    retval.AppendChild(XMLElement("specials_freq", lexical_cast<string>(SpecialsFrequency())));
    return retval;
}

void GalaxySetupPanel::Disable(bool b/* = true*/)
{
    for (std::list<GG::Wnd*>::const_iterator it = Children().begin(); it != Children().end(); ++it) {
        static_cast<GG::Control*>(*it)->Disable(b);
    }
}

void GalaxySetupPanel::SetFromXML(const XMLElement& elem)
{
    using boost::lexical_cast;
    m_stars_spin->SetValue(lexical_cast<int>(elem.Child("size").Text()));
    m_galaxy_shapes_list->Select(lexical_cast<int>(elem.Child("shape").Text()));
    m_galaxy_ages_list->Select(lexical_cast<int>(elem.Child("age").Text()));
    m_starlane_freq_list->Select(lexical_cast<int>(elem.Child("starlane_freq").Text()) - (ALLOW_NO_STARLANES ? 0 : 1));
    m_planet_density_list->Select(lexical_cast<int>(elem.Child("planet_density").Text()));
    m_specials_freq_list->Select(lexical_cast<int>(elem.Child("specials_freq").Text()));
}

void GalaxySetupPanel::Init()
{
    AttachSignalChildren();

    GG::Connect(m_stars_spin->ValueChangedSignal, &GalaxySetupPanel::SettingChanged, this);
    GG::Connect(m_galaxy_shapes_list->SelChangedSignal, &GalaxySetupPanel::SettingChanged, this);
    GG::Connect(m_galaxy_ages_list->SelChangedSignal, &GalaxySetupPanel::SettingChanged, this);
    GG::Connect(m_starlane_freq_list->SelChangedSignal, &GalaxySetupPanel::SettingChanged, this);
    GG::Connect(m_planet_density_list->SelChangedSignal, &GalaxySetupPanel::SettingChanged, this);
    GG::Connect(m_specials_freq_list->SelChangedSignal, &GalaxySetupPanel::SettingChanged, this);
    GG::Connect(m_galaxy_shapes_list->SelChangedSignal, &GalaxySetupPanel::ShapeChanged, this);

    // create and load textures
    m_textures.clear();
    for (int i = 0; i < Universe::GALAXY_SHAPES; ++i)
        m_textures.push_back(boost::shared_ptr<GG::Texture>(new GG::Texture()));
    m_textures[Universe::SPIRAL_2]->Load(ClientUI::ART_DIR + "gp_spiral2.png");
    m_textures[Universe::SPIRAL_3]->Load(ClientUI::ART_DIR + "gp_spiral3.png");
    m_textures[Universe::SPIRAL_4]->Load(ClientUI::ART_DIR + "gp_spiral4.png");
    m_textures[Universe::CLUSTER]->Load(ClientUI::ART_DIR + "gp_cluster.png");
    m_textures[Universe::ELLIPTICAL]->Load(ClientUI::ART_DIR + "gp_elliptical.png");
    m_textures[Universe::IRREGULAR]->Load(ClientUI::ART_DIR + "gp_irregular.png");
	m_textures[Universe::RING]->Load(ClientUI::ART_DIR + "gp_ring.png");

    // fill droplists
    m_galaxy_shapes_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_2ARM")));
    m_galaxy_shapes_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_3ARM")));
    m_galaxy_shapes_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_4ARM")));
    m_galaxy_shapes_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_CLUSTER")));
    m_galaxy_shapes_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_ELLIPTICAL")));
    m_galaxy_shapes_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_IRREGULAR")));
    m_galaxy_shapes_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_RING")));

    m_galaxy_ages_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_YOUNG")));
    m_galaxy_ages_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_MATURE")));
    m_galaxy_ages_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_ANCIENT")));

    if (ALLOW_NO_STARLANES)
        m_starlane_freq_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_NONE")));
    m_starlane_freq_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_FEW")));
    m_starlane_freq_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_SOME")));
    m_starlane_freq_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_SEVERAL")));
    m_starlane_freq_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_MANY")));
    m_starlane_freq_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_VERY_MANY")));

    m_planet_density_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_LOW")));
    m_planet_density_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_MEDIUM")));
    m_planet_density_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_HIGH")));

    m_specials_freq_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_NONE")));
    m_specials_freq_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_RARE")));
    m_specials_freq_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_UNCOMMON")));
    m_specials_freq_list->Insert(new CUISimpleDropDownListRow(UserString("GSETUP_COMMON")));

    // default settings
    m_galaxy_shapes_list->Select(0);
    m_galaxy_ages_list->Select(1);
    m_starlane_freq_list->Select(ALLOW_NO_STARLANES ? 3 : 2);
    m_planet_density_list->Select(1);
    m_specials_freq_list->Select(2);
}

void GalaxySetupPanel::AttachSignalChildren()
{
    AttachChild(m_stars_spin);
    AttachChild(m_galaxy_shapes_list);
    AttachChild(m_galaxy_ages_list);
    AttachChild(m_starlane_freq_list);
    AttachChild(m_planet_density_list);
    AttachChild(m_specials_freq_list);
}

void GalaxySetupPanel::DetachSignalChildren()
{
    DetachChild(m_stars_spin);
    DetachChild(m_galaxy_shapes_list);
    DetachChild(m_galaxy_ages_list);
    DetachChild(m_starlane_freq_list);
    DetachChild(m_planet_density_list);
    DetachChild(m_specials_freq_list);
}

void GalaxySetupPanel::SettingChanged(int)
{
    SettingsChangedSignal();
}

void GalaxySetupPanel::ShapeChanged(int index)
{
    ImageChangedSignal(m_textures[index]);
}


////////////////////////////////////////////////
// GalaxySetupWnd
////////////////////////////////////////////////
GalaxySetupWnd::GalaxySetupWnd() : 
    CUIWnd(UserString("GSETUP_WINDOW_TITLE"), (HumanClientApp::GetApp()->AppWidth() - GAL_SETUP_WND_WD) / 2, 
           (HumanClientApp::GetApp()->AppHeight() - GAL_SETUP_WND_HT) / 2, GAL_SETUP_WND_WD, GAL_SETUP_WND_HT, 
           GG::CLICKABLE | GG::MODAL),
    m_ended_with_ok(false),
    m_galaxy_setup_panel(0),
    m_empire_name_label(0),
    m_empire_name_edit(0),
    m_empire_color_label(0),
    m_empire_color_selector(0),
    m_preview_image(0),
    m_ok(0),
    m_cancel(0)
{
    TempUISoundDisabler sound_disabler;

    m_galaxy_setup_panel = new GalaxySetupPanel(0, 4);

    boost::shared_ptr<GG::Font> font = GG::GUI::GetGUI()->GetFont(ClientUI::FONT, ClientUI::PTS);

    const int LABELS_WIDTH = (GalaxySetupPanel::DEFAULT_WIDTH - 5) / 2;
    m_empire_color_label = new GG::TextControl(CONTROL_MARGIN, m_galaxy_setup_panel->LowerRight().y + PANEL_CONTROL_SPACING, LABELS_WIDTH, CONTROL_HEIGHT, UserString("GSETUP_EMPIRE_COLOR"), font, ClientUI::TEXT_COLOR, GG::TF_RIGHT);
    m_empire_color_selector = new EmpireColorSelector(ClientUI::PTS + 4);
    m_empire_color_selector->MoveTo(GG::Pt(LABELS_WIDTH + 2 * CONTROL_MARGIN, m_galaxy_setup_panel->LowerRight().y + PANEL_CONTROL_SPACING + (PANEL_CONTROL_SPACING - m_empire_color_selector->Height()) / 2));
    m_empire_color_selector->Select(0);
    m_empire_name_label = new GG::TextControl(CONTROL_MARGIN, m_galaxy_setup_panel->LowerRight().y, LABELS_WIDTH, m_empire_color_selector->Height(), UserString("GSETUP_EMPIRE_NAME"), font, ClientUI::TEXT_COLOR, GG::TF_RIGHT);
    m_empire_name_edit = new CUIEdit(LABELS_WIDTH + 2 * CONTROL_MARGIN, m_galaxy_setup_panel->LowerRight().y,
                                     LABELS_WIDTH, "Human");
    m_empire_name_label->OffsetMove(GG::Pt(0, (PANEL_CONTROL_SPACING - m_empire_name_label->Height()) / 2));
    m_empire_name_edit->OffsetMove(GG::Pt(0, (PANEL_CONTROL_SPACING - m_empire_name_edit->Height()) / 2));

    m_preview_ul = GG::Pt(ClientWidth() - PREVIEW_SZ.x - 7, 7);

    // create a temporary texture and static graphic
    boost::shared_ptr<GG::Texture> temp_tex(new GG::Texture());
    m_preview_image =  new GG::StaticGraphic(m_preview_ul.x, m_preview_ul.y, PREVIEW_SZ.x, PREVIEW_SZ.y, temp_tex, GG::GR_FITGRAPHIC); // create a blank graphic

    m_ok = new CUIButton(10, m_empire_color_selector->LowerRight().y + 10, 75, UserString("OK"));
    m_cancel = new CUIButton(10 + m_ok->Size().x + 15, m_empire_color_selector->LowerRight().y + 10, 75, UserString("CANCEL"));

    Init();
}

const std::string& GalaxySetupWnd::EmpireName() const
{
    return m_empire_name_edit->WindowText();
}

GG::Clr GalaxySetupWnd::EmpireColor() const
{
    return m_empire_color_selector->CurrentColor();
}

void GalaxySetupWnd::Render()
{
    CUIWnd::Render();
    GG::FlatRectangle(ClientUpperLeft().x + m_preview_ul.x - 2, ClientUpperLeft().y + m_preview_ul.y - 2, ClientUpperLeft().x + m_preview_ul.x + PREVIEW_SZ.x + 2, 
                      ClientUpperLeft().y + m_preview_ul.y + PREVIEW_SZ.y + 2, GG::CLR_BLACK, ClientUI::WND_INNER_BORDER_COLOR, 1);
}

void GalaxySetupWnd::KeyPress (GG::Key key, Uint32 key_mods)
{
    if (!m_ok->Disabled() && (key == GG::GGK_RETURN || key == GG::GGK_KP_ENTER)) // Same behaviour as if "OK" was pressed
        OkClicked();
    else if (key == GG::GGK_ESCAPE) // Same behaviour as if "Cancel" was pressed
        CancelClicked();
}

void GalaxySetupWnd::Init()
{
    AttachSignalChildren();

    GG::Connect(m_galaxy_setup_panel->ImageChangedSignal, &GalaxySetupWnd::PreviewImageChanged, this);
    GG::Connect(m_empire_name_edit->EditedSignal, &GalaxySetupWnd::EmpireNameChanged, this);
    GG::Connect(m_ok->ClickedSignal, &GalaxySetupWnd::OkClicked, this);
    GG::Connect(m_cancel->ClickedSignal, &GalaxySetupWnd::CancelClicked, this);

    PreviewImageChanged(m_galaxy_setup_panel->PreviewImage());
}

void GalaxySetupWnd::AttachSignalChildren()
{
    AttachChild(m_galaxy_setup_panel);
    AttachChild(m_empire_name_label);
    AttachChild(m_empire_name_edit);
    AttachChild(m_empire_color_label);
    AttachChild(m_empire_color_selector);
    AttachChild(m_preview_image);
    AttachChild(m_ok);
    AttachChild(m_cancel);
}

void GalaxySetupWnd::DetachSignalChildren()
{
    DetachChild(m_galaxy_setup_panel);
    DetachChild(m_empire_name_label);
    DetachChild(m_empire_name_edit);
    DetachChild(m_empire_color_label);
    DetachChild(m_empire_color_selector);
    DetachChild(m_preview_image);
    DetachChild(m_ok);
    DetachChild(m_cancel);
}

void GalaxySetupWnd::PreviewImageChanged(boost::shared_ptr<GG::Texture> new_image)
{
    if (m_preview_image) {
        DeleteChild(m_preview_image);
        m_preview_image = 0;
    }
    m_preview_image = new GG::StaticGraphic(m_preview_ul.x, m_preview_ul.y, PREVIEW_SZ.x, PREVIEW_SZ.y, new_image, GG::GR_FITGRAPHIC);
    AttachChild(m_preview_image);
}

void GalaxySetupWnd::EmpireNameChanged(const std::string& name)
{
    m_ok->Disable(name.empty());
}

void GalaxySetupWnd::OkClicked()
{
    m_ended_with_ok = true;
    m_done = true;
}

void GalaxySetupWnd::CancelClicked()
{
    m_ended_with_ok = false;
    m_done = true;
}
